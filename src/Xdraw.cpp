#include "../include/Xdraw.hpp"

#include <X11/Xlib.h>


void Draw::redraw() const {
    XClearWindow(dpy.get(), win);

    // TODO: draw

    XFlush(dpy.get());
}
