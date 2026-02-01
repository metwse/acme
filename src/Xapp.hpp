/**
 * @file Xapp.hpp
 * @brief Simulation driver interface for X11.
 */

#ifndef XAPP_HPP
#define XAPP_HPP


#include <X11/Xlib.h>

#include <memory>
#include <thread>


class App;


/**
 * @class EvLoop
 * @brief Manages the dedicated event processing thread.
 */
class EvLoop {
public:
    /** @brief Constructs an event loop associated with a specific App. */
    EvLoop(App *app_)
        : app { app_ } {}

    /** @brief Default destructor relies on jthread for cleanup. */
    ~EvLoop() = default;

    /** @brief Spawns the worker thread and begins execution. */
    void start();


private:
    /** @brief The entry point for the thread. */
    void run();

    App *app;
    std::jthread evloop;
};

/**
 * @class App
 * @brief Main Application orchestrator and X11 resource manager.
 */
class App {
public:
    App() :
        dpy { std::unique_ptr<Display, DisplayDeleter>(XOpenDisplay(NULL)) },
        evloop { this },
        scr { XDefaultScreenOfDisplay(dpy.get()) },
        root { XRootWindowOfScreen(scr) } {}

    /** @brief Default destructor relies on RAII for member cleanup. */
    ~App() = default;

    /** @brief Performs window creation and thread setup. */
    void init();

private:
    friend EvLoop;

    /** @brief Function object for deleting display. */
    class DisplayDeleter {
    public:
        void operator()(Display* dpy) const {
            if (dpy)
                XCloseDisplay(dpy);
        }
    };

    std::unique_ptr<Display, DisplayDeleter> dpy;

    EvLoop evloop;

    Screen *scr;
    Window root;
    Window win;

    static const int WIDTH = 512;
    static const int HEIGHT = 384;
};


#endif
