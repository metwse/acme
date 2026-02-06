#include "../include/Xdraw.hpp"

#include <X11/Xlib.h>


Draw::Draw(auto dpy_, auto scr_, auto win_, auto intr_, auto lex_)
  : dpy { dpy_ }, scr { scr_ }, win { win_ },
    gc { XDefaultGCOfScreen(scr) },
    intr { intr_ }, lex { lex_ } {};

void Draw::redraw() const {
    XClearWindow(dpy.get(), win);

    // TODO: draw

    XFlush(dpy.get());
}
