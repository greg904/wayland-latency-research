# Make sure that the shell is the same everywhere.
SHELL = /bin/sh

src_c := src/main.c
objs := $(src_c:%.c=%.o)

proto_objs := protocol/xdg-shell-protocol.o
proto_inc := protocol/xdg-shell-client-protocol.h

WAYLAND_PROTOCOLS_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER = $(shell pkg-config --variable=wayland_scanner wayland-scanner)

XDG_SHELL_PROTOCOL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

CFLAGS = $(shell pkg-config --cflags wayland-client wayland-cursor) -std=gnu11 -flto -O2 -Werror
LDLIBS = $(shell pkg-config --libs wayland-client wayland-cursor)
LDFLAGS =

.PHONY: all
all: example-app

.PHONY: clean
clean:
	rm -f $(objs) $(proto_objs) $(proto_inc) example-app

.PHONY: format
format:
	clang-format -i $(src_c)

###
# Wayland protocols
###

protocol/xdg-shell-client-protocol.h: $(XDG_SHELL_PROTOCOL)
	$(WAYLAND_SCANNER) client-header $< $@

protocol/xdg-shell-protocol.c: $(XDG_SHELL_PROTOCOL)
	$(WAYLAND_SCANNER) private-code $< $@

# Add dependency for prototype headers.
$(objs): $(proto_inc)

###
# Compilation
###

%.o: %.c
	$(CC) $< -c -o $@ -Iprotocol $(CPPFLAGS) $(CFLAGS)

example-app: $(objs) $(proto_objs)
	$(CC) $^ -o $@ $(CFLAGS) $(LDLIBS) $(LDFLAGS)
