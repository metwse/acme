/**
 * @file Xdraw.hpp
 * @brief Simulation visualizer
 */

#ifndef XDRAW_HPP
#define XDRAW_HPP

#include <X11/Xlib.h>

#include <memory>

class EvLoop /* defined in Xapp.hpp */;
class Interpreter /* defined in interpreter.hpp */;
class Lex /* defined in lex.hpp */;


/** @brief Draws the simulatoin into X graphics context */
class Draw {
public:
    Draw(auto dpy_, auto scr_, auto win_, auto intr_, auto lex_);

    void redraw() const;

private:
    std::shared_ptr<Display> dpy;

    Screen *scr;
    Window root;
    Window win;

    GC gc;

    std::shared_ptr<Interpreter> intr;
    std::shared_ptr<Lex> lex;
};


#endif
