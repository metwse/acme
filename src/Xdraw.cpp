#include "../include/Xdraw.hpp"
#include "../include/interpreter.hpp"
#include "../include/core.hpp"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <vector>

using std::vector;
using std::cerr;
using std::unique_ptr;


#define p(x_, y_) scale_x(x_), scale_y(y_)

void Draw::redraw() const {
    XClearWindow(dpy.get(), win);

    XSetLineAttributes(dpy.get(), gc, (scale + 4) / 4, LineSolid, CapRound, JoinRound);

    for (const auto &wire : intr->wires) {
        XSetForeground(dpy.get(), gc,
                       wire.second.state ? active_color.pixel : inactive_color.pixel);

        try {
            draw(wire.second);
        } catch (std::bad_cast &) {
            cerr << "Warning: Skipping wire " << lex->ident_name(wire.second.id)
                << " has invalid metadata type.\n";
        } catch (std::out_of_range &) {
            cerr << "Warning: Skipping wire " << lex->ident_name(wire.second.id)
                << " missing required metadata.\n";
        }
    }

    for (const auto &unit : intr->units) {
        XSetForeground(dpy.get(), gc, inactive_color.pixel);

        try {
            draw(unit.second);
        } catch (std::bad_cast &) {
            cerr << "Warning: Skipping unit " << lex->ident_name(unit.second.id)
                << " has invalid metadata type.\n";
        } catch (std::out_of_range &) {
            cerr << "Warning: Skipping unit " << lex->ident_name(unit.second.id)
                << " missing required metadata.\n";
        }
    }

    XFlush(dpy.get());
}

void Draw::draw(const Lut &lut, int x, int y) const {
    auto &shape = dynamic_cast<const TVPath &>(lut.table.get(k_shape));

    for (auto &path : shape.paths) {
        vector<XPoint> points;

        for (auto &point : path) {
            auto &num_point = dynamic_cast<const TVPointNum &>(*point.get());

            points.push_back(XPoint(p(x + num_point.x, y + num_point.y)));
        }

        XDrawLines(dpy.get(), win, gc,
                   &points[0], points.size(), CoordModeOrigin);
    }
}

void Draw::draw(const Wire &wire,
                const vector<unique_ptr<TVPoint>> &path) const {
    vector<XPoint> points;

    size_t i = 0;
    for (auto &point : path) {
        auto ident_point_ptr = dynamic_cast<const TVPointIdent *>(point.get());

        if (ident_point_ptr != nullptr) {
            /* automatically connect wire to unit's port if an ident present
             * in path*/
            auto ident_point = &*ident_point_ptr;

            /* unit wire connected to and its lut */
            auto &unit = intr->units.at(ident_point->id);
            auto &lut = intr->luts.at(unit.lut_id);

            auto &unit_pos = dynamic_cast<const TVPointNum &>(unit.table.get(k_pos));

            auto get_port_position = [&](TableKeyId key, auto port_index) {
                auto &port_path = dynamic_cast<const TVPath &>(lut.table.get(key));
                auto &point = *port_path.paths[0][port_index].get();
                auto &num_point = dynamic_cast<const TVPointNum &>(point);
                points.push_back(XPoint(p(num_point.x + unit_pos.x,
                                          num_point.y + unit_pos.y)));
            };

            for (size_t i = 0; i < unit.input_wires.size(); i++)
                if (unit.input_wires[i] == wire.id)
                    get_port_position(k_input, i);

            for (size_t i = 0; i < unit.output_wires.size(); i++)
                if (unit.output_wires[i] == wire.id)
                    get_port_position(k_output, i);
        } else {
            /* add point otherwise */
            auto &num_point = dynamic_cast<const TVPointNum &>(*point.get());

            points.push_back(XPoint(p(num_point.x, num_point.y)));

            if (i == 0 || i == path.size() - 1) {
                XFillArc(dpy.get(), win, gc,
                         p(num_point.x - 0.5, num_point.y - 0.5),
                         scale, scale, 0, 360 * 64);
            }
        }

        i++;
    }

    XDrawLines(dpy.get(), win, gc, &points[0], points.size(), CoordModeOrigin);
}

void Draw::draw(const Wire &wire) const {
    auto &paths = dynamic_cast<const TVPath &>(wire.table.get(k_path));

    for (auto &path : paths.paths)
        draw(wire, path);
}

void Draw::draw(const Unit &unit) const {
    auto &pos = dynamic_cast<const TVPointNum &>(unit.table.get(k_pos));

    draw(intr->luts.at(unit.lut_id),
         static_cast<int>(pos.x), static_cast<int>(pos.y));
}

void Draw::init_key_ids() {
    k_shape = lex->get_ident_id("_shape");
    k_input = lex->get_ident_id("_input");
    k_output = lex->get_ident_id("_output");
    k_path = lex->get_ident_id("_path");
    k_pos = lex->get_ident_id("_pos");
}
