/* For memfd_create. */
#define _GNU_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include <wayland-cursor.h>

#include "xdg-shell-client-protocol.h"

static const int width = 500;
static const int height = 500;

/* Wayland globals. */
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct xdg_wm_base *wm_base = NULL;

/* Surface variables. */
static struct wl_surface *surface;
static struct wl_buffer *surface_buffer;
static uint32_t *pixels;

/* Cursor variables. */
static struct wl_surface *cursor_surface;

static bool running = true;

struct sw_cursor {
    bool is_visible;
    int curr_x;
    int curr_y;
};

static struct sw_cursor immediate_cursor;
static struct sw_cursor frame_cursor;

static bool requested_frame = false;
static int frame_cursor_x;
static int frame_cursor_y;

static void clamp(int *val, int min, int max)
{
    if (*val < min) {
        *val = min;
    } else if (*val > max) {
        *val = max;
    }
}

static void xor_rect(int x_min, int y_min, int x_max, int y_max, uint32_t mask)
{
    clamp(&x_min, 0, width);
    clamp(&y_min, 0, height);
    clamp(&x_max, 0, width);
    clamp(&y_max, 0, height);

    if (x_min >= x_max || y_min >= y_max)
        return;

    for (int y = y_min; y <= y_max; ++y) {
        for (int x = x_min; x <= x_max; ++x) {
            pixels[y * width + x] ^= mask;
        }
    }

    wl_surface_damage_buffer(surface, x_min, y_min, x_max - x_min + 1,
                             y_max - y_min + 1);
}

static void sw_cursor_move(struct sw_cursor *cursor, int new_x, int new_y,
                           uint32_t mask)
{
    if (cursor->is_visible) {
        if (new_x == cursor->curr_x && new_y == cursor->curr_y)
            return;

        /* Erase the rectangle at the old location. */
        xor_rect(cursor->curr_x - 5, cursor->curr_y - 5, cursor->curr_x + 5,
                 cursor->curr_y + 5, mask);
    }

    /* Draw a rectangle at the new location. */
    xor_rect(new_x - 5, new_y - 5, new_x + 5, new_y + 5, mask);

    cursor->curr_x = new_x;
    cursor->curr_y = new_y;
    cursor->is_visible = true;

    /* Display the new buffer. */
    wl_surface_attach(surface, surface_buffer, 0, 0);
    wl_surface_commit(surface);
}

static void global_add_handler(void *data, struct wl_registry *registry,
                               uint32_t name, const char *interface,
                               uint32_t version)
{
    if (!strcmp(interface, "wl_compositor")) {
        compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (!strcmp(interface, "wl_shm")) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (!strcmp(interface, "wl_seat")) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    } else if (!strcmp(interface, "xdg_wm_base")) {
        wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}

static void global_remove_handler(void *data, struct wl_registry *registry,
                                  uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
    .global = global_add_handler,
    .global_remove = global_remove_handler,
};

static void frame_done_handler(void *data, struct wl_callback *callback,
                               uint32_t callback_data)
{
    sw_cursor_move(&frame_cursor, frame_cursor_x, frame_cursor_y, 0x0000FF00);
    requested_frame = false;
}

static const struct wl_callback_listener frame_listener = {
    .done = frame_done_handler,
};

static void pointer_enter_handler(void *data, struct wl_pointer *pointer,
                                  uint32_t serial, struct wl_surface *surface,
                                  wl_fixed_t x, wl_fixed_t y)
{
    wl_pointer_set_cursor(pointer, serial, cursor_surface, 5, 5);
}

static void pointer_leave_handler(void *data, struct wl_pointer *pointer,
                                  uint32_t serial, struct wl_surface *surface)
{
}

static void pointer_motion_handler(void *data, struct wl_pointer *pointer,
                                   uint32_t time, wl_fixed_t surface_x,
                                   wl_fixed_t surface_y)
{
    int int_x = wl_fixed_to_int(surface_x);
    int int_y = wl_fixed_to_int(surface_y);

    /* Update immediate cursor directly. */
    sw_cursor_move(&immediate_cursor, int_x, int_y, 0x00FF0000);

    /* Request a frame notification to update frame cursor. */
    frame_cursor_x = int_x;
    frame_cursor_y = int_y;
    if (!requested_frame) {
        struct wl_callback *frame_callback = wl_surface_frame(surface);
        wl_callback_add_listener(frame_callback, &frame_listener, NULL);
        wl_surface_commit(surface);
        requested_frame = true;
    }
}

static void pointer_button_handler(void *data, struct wl_pointer *pointer,
                                   uint32_t serial, uint32_t time,
                                   uint32_t button, uint32_t state)
{
}

static void pointer_axis_handler(void *data, struct wl_pointer *pointer,
                                 uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter_handler,
    .leave = pointer_leave_handler,
    .motion = pointer_motion_handler,
    .button = pointer_button_handler,
    .axis = pointer_axis_handler};

static void xdg_surface_configure_handler(void *data,
                                          struct xdg_surface *xdg_surface,
                                          uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler};

static void xdg_toplevel_configure_handler(void *data,
                                           struct xdg_toplevel *xdg_toplevel,
                                           int32_t width, int32_t height,
                                           struct wl_array *states)
{
}

static void xdg_toplevel_close_handler(void *data,
                                       struct xdg_toplevel *xdg_toplevel)
{
    running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler};

int main(int argc, char **argv)
{
    /* Connect to Wayland server and retreive globals. */
    struct wl_display *display = wl_display_connect(NULL);
    assert(display);
    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_dispatch(display);
    assert(compositor);
    assert(shm);
    assert(seat);
    assert(wm_base);

    /* Create a shared memory pool for the surface's buffer and for the
     * cursor's surface's buffer. */
    int memfd_size = (width * height + 11 * 11) * 4;
    int memfd = memfd_create("buffer", 0);
    assert(memfd != -1);
    assert(ftruncate(memfd, memfd_size) != -1);
    pixels = mmap(NULL, memfd_size, PROT_WRITE, MAP_SHARED, memfd, 0);
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, memfd, memfd_size);

    /* Create a buffer and a surface for the cursor. */
    for (int i = 0; i < 11 * 11; ++i)
        pixels[height * width + i] = 0x00FFFFFF;
    struct wl_buffer *cursor_buffer = wl_shm_pool_create_buffer(
        pool, height * width * 4, 11, 11, 11 * 4, WL_SHM_FORMAT_XRGB8888);
    cursor_surface = wl_compositor_create_surface(compositor);
    wl_surface_attach(cursor_surface, cursor_buffer, 0, 0);
    wl_surface_commit(cursor_surface);

    /* Setup pointer. */
    struct wl_pointer *pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);

    /* Create toplevel surface. */
    surface = wl_compositor_create_surface(compositor);
    struct xdg_surface *xdg_surf =
        xdg_wm_base_get_xdg_surface(wm_base, surface);
    xdg_surface_add_listener(xdg_surf, &xdg_surface_listener, NULL);
    struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surf);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

    /* Wait for the surface to be configured. */
    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    /* Create a buffer and attach it to the surface. */
    surface_buffer = wl_shm_pool_create_buffer(
        pool, 0, width, height, width * 4, WL_SHM_FORMAT_XRGB8888);
    wl_surface_attach(surface, surface_buffer, 0, 0);
    wl_surface_commit(surface);

    /* Main loop: dispatch events. */
    while (running)
        assert(wl_display_dispatch(display) != -1);

    return EXIT_SUCCESS;
}
