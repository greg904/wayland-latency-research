# wayland-latency-research

The goal of this project is to measure the latency of two different drawing
methods on my Wayland compositor.

The `example-app` directory contains code for a simple application that draws a
rectangle following the mouse cursor in two ways :
1. As soon as a mouse event is received, it moves the rectangle on the CPU
buffer.
2. When a mouse event is received, it requests a frame event from the
compositor and when that event is received, it moves the rectangle on the CPU
buffer.

Obviously, the second way will lag behind, but the goal is to make it lag less
because it is the way most applications use to render.

A possible optimisation is that the compositor sends the frame event sooner.
This was [implemented in sway](https://github.com/swaywm/sway/pull/4588) with
the `max_render_time` option. You can try to change that option to see how it
affects latency.
