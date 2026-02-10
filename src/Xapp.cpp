#include "../include/Xapp.hpp"
#include "../include/Xeditor.hpp"
#include "../include/interpreter.hpp"
#include "../include/lex.hpp"
#include "../include/grammar.hpp"
#include "../include/table.hpp"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using std::jthread;
using std::cerr;


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
        try {
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
        } catch (std::exception &) {
            cerr << "Could not toggle wire!\n";
        }
    }
}

void EvLoop::run() {
    Atom wm_delete_win = XInternAtom(dpy.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy.get(), win, &wm_delete_win, 1);

    XSelectInput(dpy.get(), win,
                 ExposureMask | ButtonPressMask |
                 KeyPressMask | KeyReleaseMask |
                 StructureNotifyMask);

    XEvent ev;
    bool quit = false;
    bool ctrl_hold = false;

    draw.redraw();

    while (!quit) {
        XNextEvent(dpy.get(), &ev);

        switch (ev.type) {
        case Expose:
            break;

        case ConfigureNotify:
            editor->set_viewport(ev.xconfigure.width, ev.xconfigure.height);
            break;

        case KeyPress:
            if (editor_mode) {
                /* check for Tab to exit editor mode */
                {
                    KeySym ksym = XLookupKeysym(&ev.xkey, 0);
                    if (ksym == XK_Tab) {
                        editor_mode = false;
                        break;
                    }
                }
                bool save_requested = editor->handle_key(ev.xkey);
                if (save_requested) {
                    if (editor->save()) {
                        reload_circuit();
                    }
                }
                break;
            }

            switch (ev.xkey.keycode) {
            case 37: // CTRL_L
            case 105: // CTRL_R
                ctrl_hold = true;
                break;
            case 23: // Tab
                editor_mode = true;
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
            if (editor_mode) {
                editor->handle_button(ev.xbutton);
                break;
            }

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

        if (editor_mode)
            editor->draw();
        else
            draw.redraw();
    }
}

void EvLoop::reload_circuit() {
    std::string text = editor->content();
    std::istringstream file_stream(text);

    auto lex = std::make_shared<Lex>(file_stream);
    auto new_intr = std::make_shared<Interpreter>(global_cfg()->new_parser());

    enum rdesc_result res;
    bool success = true;
    while (true) {
        auto tk = lex->next();

        if (tk.id == TK_NOTOKEN) {
            cerr << "Editor reload: syntax error\n";
            success = false;
            break;
        } else if (tk.id == TK_EOF) {
            break;
        }

        res = new_intr->pump(tk);
        if (res == RDESC_NOMATCH) {
            cerr << "Editor reload: syntax error in statement\n";
            success = false;
            break;
        }
    }

    if (success) {
        /* swap interpreter and rebuild simulation */
        intr_FOR_RC = new_intr;
        draw.intr = new_intr;
        draw.lex = lex;
        draw.init_key_ids();
        sim.~Simulation();
        new (&sim) Simulation(*new_intr.get());
        k_path = lex->get_ident_id("_path");
        cerr << "Editor reload: success\n";
    }
}
