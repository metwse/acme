#include "../include/Xapp.hpp"
#include "../include/interpreter.hpp"
#include "../include/table.hpp"

#include <X11/X.h>
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

void EvLoop::toggle_wire(int x, int y) {
    for (auto &wire : sim.wires) {
        auto &path =
            dynamic_cast<const TVPath &>(wire.second.table.get(k_path));

        auto *tip_ = dynamic_cast<const TVPointNum *>(path.paths[0][0].get());

        if (tip_ != nullptr) {
            auto &tip = *tip_;

            if (x - draw.scale <= draw.scale_x(tip.x) &&
                draw.scale_x(tip.x) <= x + draw.scale &&
                y - draw.scale <= draw.scale_y(tip.y) &&
                draw.scale_y(tip.y) <= y + draw.scale
            ) {
                sim.set_wire_state(wire.first, !wire.second.state);
                sim.stabilize();
            }
        }
    }
}

void EvLoop::run() {
    Atom wm_delete_win = XInternAtom(dpy.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy.get(), win, &wm_delete_win, 1);

    XSelectInput(dpy.get(), win,
                 ExposureMask | ButtonPressMask |
                 KeyPressMask | KeyReleaseMask);

    XEvent ev;
    bool quit = false;
    bool ctrl_hold = false;

    draw.redraw();

    while (!quit) {
        XNextEvent(dpy.get(), &ev);

        switch (ev.type) {
        case Expose:
            break;

        case KeyPress:
            switch (ev.xkey.keycode) {
            case 37: // CTRL_L
            case 105: // CTRL_R
                ctrl_hold = true;
                break;
            case 19: // 0
                draw.offset_x -= ev.xkey.x;
                draw.offset_x *= 10 / draw.scale;
                draw.offset_x += ev.xkey.x;
                draw.offset_y -= ev.xkey.y;
                draw.offset_y *= 10 / draw.scale;
                draw.offset_y += ev.xkey.y;
                draw.scale = 10;
                break;
            default:
                break;
            }
            break;

        case KeyRelease:
            switch (ev.xkey.keycode) {
            case 37: // CTRL_L
            case 105: // CTRL_R
                ctrl_hold = false;
                break;
            }
            break;

        case ButtonPress:
            if (ev.xbutton.button == 1) {
                toggle_wire(ev.xbutton.x, ev.xbutton.y);
                break;
            }
            if (ctrl_hold) {
                switch (ev.xbutton.button) {
                case (4):
                        draw.offset_x -= ev.xbutton.x;
                        draw.offset_x *= 1.25;
                        draw.offset_x += ev.xbutton.x;
                        draw.offset_y -= ev.xbutton.y;
                        draw.offset_y *= 1.25;
                        draw.offset_y += ev.xbutton.y;

                        draw.scale *= 1.25;
                        break;
                case (5):
                        draw.offset_x -= ev.xbutton.x;
                        draw.offset_x *= 0.8;
                        draw.offset_x += ev.xbutton.x;
                        draw.offset_y -= ev.xbutton.y;
                        draw.offset_y *= 0.8;
                        draw.offset_y += ev.xbutton.y;

                        draw.scale *= 0.8;
                        break;
                default: break;
                }
            } else {
                switch (ev.xbutton.button) {
                case (4):
                        draw.offset_y += draw.scale;
                        break;
                case (5):
                        draw.offset_y -= draw.scale;
                        break;
                case (6):
                        draw.offset_x += draw.scale;
                        break;
                case (7):
                        draw.offset_x -= draw.scale;
                        break;
                default: break;
                }
            }
            break;

        case ClientMessage:
            if (Atom(ev.xclient.data.l[0]) == wm_delete_win)
                quit = true;
            break;

        default: break;  // GCOVR_EXCL_LINE
        }

        draw.redraw();
    }
}
