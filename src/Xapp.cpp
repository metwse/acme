#include "Xapp.hpp"
#include "detail.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <thread>

using std::jthread;


void App::init() {
    Colormap border_color = XBlackPixelOfScreen(scr);
    Colormap background_color = XWhitePixelOfScreen(scr);

    int x = XWidthOfScreen(scr) / 2 - WIDTH / 2;
    int y = XHeightOfScreen(scr) / 2 - HEIGHT / 2;

    win = XCreateSimpleWindow(dpy.get(), root, x, y, WIDTH, HEIGHT, 1,
                              border_color, background_color);

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
    Atom wm_delete_win = XInternAtom(app->dpy.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(app->dpy.get(), app->win, &wm_delete_win, 1);

    XSelectInput(app->dpy.get(), app->win, ExposureMask);

    XEvent ev;
    bool quit = false;

    while (!quit) {
        XNextEvent(app->dpy.get(), &ev);

        switch (ev.type) {
            case Expose:
                break;

            case ClientMessage:
                if (Atom(ev.xclient.data.l[0]) == wm_delete_win)
                    quit = true;
                break;

            default: unreachable();  // GCOVR_EXCL_LINE
        }
    }
}
