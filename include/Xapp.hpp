/**
 * @file Xapp.hpp
 * @brief Simulation interface for X11.
 */

#ifndef XAPP_HPP
#define XAPP_HPP

#include "Xdraw.hpp"
#include "interpreter.hpp"

#include <X11/Xlib.h>

#include <memory>
#include <thread>

class Lex /* defined in lex.hpp */;


class App;

Window create_window(App *app);

/** @brief Manages the dedicated event processing thread. */
class EvLoop {
public:
    /** @brief Constructs an event loop associated with a specific App. */
    EvLoop(App *app);

    /** @brief Default destructor relies on jthread for cleanup. */
    ~EvLoop() = default;

    /** @brief Spawns the worker thread and begins execution. */
    void start();

private:
    /** @brief The entry point for the thread. */
    void run();

    void toggle_wire(int x, int y);

    std::shared_ptr<Interpreter> intr_FOR_RC;
    Simulation sim;

    std::shared_ptr<Display> dpy;

    Window win;

    Draw draw;

    std::jthread evloop;

    TableKeyId k_path;
};

/** @brief Main Application orchestrator and X11 resource manager. */
class App {
private:
    /** @brief Function object for deleting display. */
    class DisplayDeleter {
    public:
        void operator()(Display *dpy) const {
            if (dpy)
                XCloseDisplay(dpy);
        }
    };

public:
    App(auto intr_, auto lex_) :
        intr { intr_ }, lex { lex_ },
        dpy { std::shared_ptr<Display>(XOpenDisplay(NULL),
                                       App::DisplayDeleter {}) },
        scr { XDefaultScreenOfDisplay(dpy.get()) },
        root { XRootWindowOfScreen(scr) },
        win { create_window(this) },
        evloop { this } {}

    /** @brief Default destructor relies on RAII for member cleanup. */
    ~App() = default;

    /** @brief Performs window creation and thread setup. */
    void init();

private:
    friend Window create_window(App *app);
    friend EvLoop;
    friend Draw;

    std::shared_ptr<Interpreter> intr;
    std::shared_ptr<Lex> lex;

    std::shared_ptr<Display> dpy;

    Screen *scr;
    Window root;
    Window win;

    EvLoop evloop;

    static const int WIDTH = 512;
    static const int HEIGHT = 384;
};


inline EvLoop::EvLoop(App *app)
    : intr_FOR_RC { app->intr }, sim { *app->intr.get() },
      dpy { app->dpy }, win { app->win },
      draw { dpy, app->scr, win, app->intr, app->lex },
      k_path { app->lex->get_ident_id("_path") } {}


#endif
