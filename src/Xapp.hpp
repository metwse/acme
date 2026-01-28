/**
 * @file Xapp.hpp
 * @brief Simulation driver interface for X11.
 */

#ifndef XDRIVER_HPP
#define XDRIVER_HPP


#include <X11/Xlib.h>

#include <thread>


class App;

/**
 * @class XConnection
 * @brief RAII wrapper for the X11 Display connection.
 */
class XConnection {
public:
    /** @brief Opens a connection to the default X display. */
    XConnection()
        : dpy { XOpenDisplay(NULL) } {};

    /** @brief Closes the X display connection. */
    ~XConnection()
        { XCloseDisplay(dpy); }

    Display *dpy /**< Pointer to the X11 Display structure. */;
};

/**
 * @class EvLoop
 * @brief Manages the dedicated event processing thread.
 */
class EvLoop {
public:
    /** @brief Constructs an event loop associated with a specific App. */
    EvLoop(App *app_)
        : app { app_ } {}

    /** @brief Ensures the worker thread is joined before destruction. */
    ~EvLoop()
        { evloop.join(); };

    /** @brief Spawns the worker thread and begins execution. */
    void start();


private:
    /** @brief The entry point for the thread. */
    void run();

    App *app;
    std::thread evloop;
};

/**
 * @class App
 * @brief Main Application orchestrator and X11 resource manager.
 */
class App : public XConnection {
public:
    App() :
        evloop { this }, scr { XDefaultScreenOfDisplay(dpy) },
        root { XRootWindowOfScreen(scr) } {}

    /** @brief Default destructor relies on RAII for member cleanup. */
    ~App() = default;

    /** @brief Performs window creation and thread setup. */
    void init();

private:
    friend EvLoop;

    EvLoop evloop;

    Screen *scr;
    Window root;
    Window win;

    static const int WIDTH = 512;
    static const int HEIGHT = 384;
};


#endif
