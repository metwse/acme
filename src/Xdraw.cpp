#include "../include/Xdraw.hpp"
#include "../include/interpreter.hpp"
#include "../include/core.hpp"

#include <X11/Xlib.h>

#include <memory>
#include <utility>
#include <vector>

using std::vector;


#define p(x_, y_) offset_x + (x_) * scale, offset_y + (y_) * scale

void Draw::redraw() const {
    XClearWindow(dpy.get(), win);

    for (const auto &wire : intr->wires)
        draw(wire.second);

    for (const auto &unit : intr->units)
        draw(unit.second);

    XFlush(dpy.get());
}

void Draw::draw(const Lut &lut, int x, int y) const {
    auto &shape = dynamic_cast<const TVPath &>(lut.table.get(k_shape));

    for (auto &path : shape.paths) {
        vector<XPoint> points;

        for (auto &point_ : path) {
            auto &point = dynamic_cast<const TVPointNum &>(*point_.get());

            points.push_back(XPoint(p(x + point.x, y + point.y)));
        }

        XDrawLines(dpy.get(), win, gc,
                   &points[0], points.size(), CoordModeOrigin);
    }
}

void Draw::draw(const Wire &wire) const {
    auto &path = dynamic_cast<const TVPath &>(wire.table.get(k_path));
    vector<XPoint> points;

    for (auto &point : path.paths.at(0)) {
        auto ident_point_ = dynamic_cast<const TVPointIdent *>(point.get());

        if (ident_point_ != nullptr) {
            auto ident_point = &*ident_point_;

            auto &unit = intr->units.at(ident_point->id);
            auto &lut = intr->luts.at(unit.lut_id);

            auto &pos = dynamic_cast<const TVPointNum &>(unit.table.get(k_pos));

            for (size_t i = 0; i < unit.input_wires.size(); i++) {
                if (unit.input_wires[i] == wire.id) {
                    auto &num_point = dynamic_cast<const TVPointNum &>(
                        *dynamic_cast<const TVPath &>(
                            lut.table.get(k_input)).paths[0][i].get());

                    points.push_back(XPoint(p(num_point.x + pos.x, num_point.y + pos.y)));
                    break;
                }
            }

            for (size_t i = 0; i < unit.output_wires.size(); i++) {
                if (unit.output_wires[i] == wire.id) {
                    auto &num_point = dynamic_cast<const TVPointNum &>(
                        *dynamic_cast<const TVPath &>(
                            lut.table.get(k_output)).paths[0][i].get());

                    points.push_back(XPoint(p(num_point.x + pos.x, num_point.y + pos.y)));
                    break;
                }
            }
        } else {
            auto &num_point = dynamic_cast<const TVPointNum &>(*point.get());

            points.push_back(XPoint(p(num_point.x, num_point.y)));
        }
    }

    XDrawLines(dpy.get(), win, gc,
               &points[0], points.size(), CoordModeOrigin);
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
