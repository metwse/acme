#include "../include/Xapp.hpp"
#include "detail.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <thread>

using std::jthread;


Window create_window(App *app) {
    Colormap border_color = XBlackPixelOfScreen(app->scr);
    Colormap background_color = XWhitePixelOfScreen(app->scr);

    int x = XWidthOfScreen(app->scr) / 2 - app->WIDTH / 2;
    int y = XHeightOfScreen(app->scr) / 2 - app->HEIGHT / 2;

    return XCreateSimpleWindow(app->dpy.get(), app->root, x, y,
                               app->WIDTH, app->HEIGHT, 1,
                               border_color, background_color);

}


void App::init() {
    XSizeHints size_hints;
    size_hints.flags = PMinSize;
    size_hints.min_width = WIDTH;
    size_hints.min_height = HEIGHT;
    XSetNormalHints(dpy.get(), win, &size_hints);

    XTextProperty wm_name = {
        .value = (unsigned char *)("acme-sim"),
        .encoding = XUTF8StringStyle,
        .format = 8, .nitems = 8,
    };
    XSetWMName(dpy.get(), win, &wm_name);

    XMapWindow(dpy.get(), win);

    evloop.start();
}

void EvLoop::start() {
    evloop = jthread(&EvLoop::run, this);
}

void EvLoop::run() {
    Atom wm_delete_win = XInternAtom(dpy.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy.get(), win, &wm_delete_win, 1);

    XSelectInput(dpy.get(), win, ExposureMask);

    XEvent ev;
    bool quit = false;

    draw.redraw();

    while (!quit) {
        XNextEvent(dpy.get(), &ev);

        switch (ev.type) {
            case Expose:
                draw.redraw();
                break;

            case ClientMessage:
                if (Atom(ev.xclient.data.l[0]) == wm_delete_win)
                    quit = true;
                break;

            default: unreachable();  // GCOVR_EXCL_LINE
        }
    }
}
