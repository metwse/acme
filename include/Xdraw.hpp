/**
 * @file Xdraw.hpp
 * @brief Simulation visualizer
 */

#ifndef XDRAW_HPP
#define XDRAW_HPP


#include "table.hpp"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <memory>

class EvLoop /* defined in Xapp.hpp */;
class Interpreter /* defined in interpreter.hpp */;
class Lex /* defined in lex.hpp */;

class Lut /* defined in core.hpp */;
class Wire /* defined in core.hpp */;
class Unit /* defined in core.hpp */;


/** @brief Draws the simulatoin into X graphics context */
class Draw {
public:
    Draw(auto dpy_, auto scr_, auto win_, auto intr_, auto lex_)
        : dpy { dpy_ }, scr { scr_ }, win { win_ },
          gc { XDefaultGCOfScreen(scr) },
          intr { intr_ }, lex { lex_ } {
        Colormap cmap = XDefaultColormapOfScreen(scr);

        XParseColor(dpy.get(), cmap, "green", &active_color);
        XAllocColor(dpy.get(), cmap, &active_color);
        XParseColor(dpy.get(), cmap, "black", &inactive_color);
        XAllocColor(dpy.get(), cmap, &inactive_color);

        init_key_ids();
    }

    void draw(const Lut &, int x, int y) const;
    void draw(const Wire &) const;
    void draw(const Unit &) const;

    void redraw() const;

    int offset_x {};
    int offset_y {};
    double scale { 10 };

private:
    void init_key_ids();

    /* helper function to draw one path in a wire */
    void draw(const Wire &, const std::vector<std::unique_ptr<TVPoint>> &) const;

    std::shared_ptr<Display> dpy;

    Screen *scr;
    Window root;
    Window win;

    GC gc;

    std::shared_ptr<Interpreter> intr;
    std::shared_ptr<Lex> lex;

    TableKeyId k_shape;
    TableKeyId k_input;
    TableKeyId k_output;
    TableKeyId k_path;
    TableKeyId k_pos;

    XColor active_color;
    XColor inactive_color;
};


#endif
