#include "../include/Xeditor.hpp"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

using std::string;
using std::vector;


static vector<string> split_lines(const string &text) {
    vector<string> result;
    std::istringstream stream(text);
    string line;

    while (std::getline(stream, line))
        result.push_back(line);

    if (result.empty())
        result.push_back("");

    return result;
}


Editor::Editor(std::shared_ptr<Display> dpy_, Screen *scr_, Window win_,
               const string &content_, const string &filepath)
    : dpy { dpy_ }, scr { scr_ }, win { win_ },
      gc { XCreateGC(dpy_.get(), win_, 0, nullptr) },
      lines { split_lines(content_) },
      filepath_ { filepath }
{
    font = XLoadQueryFont(dpy.get(), "-misc-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");
    if (!font)
        font = XLoadQueryFont(dpy.get(), "fixed");

    XSetFont(dpy.get(), gc, font->fid);

    char_width = XTextWidth(font, "M", 1);
    char_ascent = font->ascent;
    char_height = font->ascent + font->descent;

    Colormap cmap = XDefaultColormapOfScreen(scr);

    auto alloc = [&](XColor &color, const char *name) {
        XParseColor(dpy.get(), cmap, name, &color);
        XAllocColor(dpy.get(), cmap, &color);
    };

    alloc(bg_color,      "#1e1e2e");
    alloc(fg_color,      "#cdd6f4");
    alloc(gutter_bg,     "#181825");
    alloc(gutter_fg,     "#6c7086");
    alloc(keyword_color, "#a6e3a1");
    alloc(cursor_color,  "#f5e0dc");
    alloc(status_bg,     "#313244");
    alloc(status_fg,     "#cdd6f4");
    alloc(scrollbar_bg,  "#181825");
    alloc(scrollbar_fg,  "#585b70");
}


void Editor::set_viewport(int w, int h) {
    vp_width = w;
    vp_height = h;
}

int visible_lines_count(int vp_height, int char_height, int status_bar_h, int pad) {
    return std::max(1, (vp_height - status_bar_h - pad) / char_height);
}


void Editor::ensure_cursor_visible() {
    int vis = visible_lines_count(vp_height, char_height, STATUS_BAR_HEIGHT, PADDING);

    if (cursor_row < scroll_y)
        scroll_y = cursor_row;

    if (cursor_row >= scroll_y + vis)
        scroll_y = cursor_row - vis + 1;

    /* horizontal scrolling */
    int gutter_px = GUTTER_CHARS * char_width;
    int text_area_chars = std::max(1, (vp_width - gutter_px - PADDING * 2) / char_width);

    if (cursor_col < scroll_x)
        scroll_x = cursor_col;

    if (cursor_col >= scroll_x + text_area_chars)
        scroll_x = cursor_col - text_area_chars + 1;
}


void Editor::draw() const {
    int gutter_px = GUTTER_CHARS * char_width;
    int vis = visible_lines_count(vp_height, char_height, STATUS_BAR_HEIGHT, PADDING);

    /* background */
    XSetForeground(dpy.get(), gc, bg_color.pixel);
    XFillRectangle(dpy.get(), win, gc, 0, 0, vp_width, vp_height);

    /* gutter background */
    XSetForeground(dpy.get(), gc, gutter_bg.pixel);
    XFillRectangle(dpy.get(), win, gc, 0, 0, gutter_px, vp_height - STATUS_BAR_HEIGHT);

    /* draw visible lines */
    for (int i = 0; i < vis && (scroll_y + i) < (int)lines.size(); i++) {
        int y = PADDING + i * char_height + char_ascent;

        draw_line_number(scroll_y + i + 1, y);
        draw_text_line(scroll_y + i, y);
    }

    draw_cursor(PADDING);
    draw_scrollbar();
    draw_status_bar();

    XFlush(dpy.get());
}


void Editor::draw_line_number(int line_num, int y) const {
    char buf[16];
    snprintf(buf, sizeof(buf), "%4d", line_num);
    int len = strlen(buf);

    XSetForeground(dpy.get(), gc, gutter_fg.pixel);
    XDrawString(dpy.get(), win, gc, PADDING, y, buf, len);
}


void Editor::draw_text_line(int line_idx, int y) const {
    int gutter_px = GUTTER_CHARS * char_width;
    const string &line = lines[line_idx];

    int text_area_w = vp_width - gutter_px - PADDING;

    /* clip region: only draw within text area */
    XRectangle clip = {
        (short)gutter_px, 0,
        (unsigned short)text_area_w,
        (unsigned short)(vp_height - STATUS_BAR_HEIGHT)
    };
    XSetClipRectangles(dpy.get(), gc, 0, 0, &clip, 1, Unsorted);

    /* tokenize for syntax highlighting */
    size_t pos = 0;
    int x = gutter_px + PADDING - scroll_x * char_width;

    while (pos < line.size()) {
        /* skip spaces */
        if (line[pos] == ' ' || line[pos] == '\t') {
            x += (line[pos] == '\t') ? char_width * 4 : char_width;
            pos++;
            continue;
        }

        /* collect a word */
        size_t word_start = pos;
        while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t')
            pos++;

        string word = line.substr(word_start, pos - word_start);

        if (is_keyword(word))
            XSetForeground(dpy.get(), gc, keyword_color.pixel);
        else
            XSetForeground(dpy.get(), gc, fg_color.pixel);

        XDrawString(dpy.get(), win, gc, x, y, word.c_str(), word.size());
        x += char_width * (int)word.size();
    }

    /* reset clip */
    XRectangle full = { 0, 0, (unsigned short)vp_width, (unsigned short)vp_height };
    XSetClipRectangles(dpy.get(), gc, 0, 0, &full, 1, Unsorted);
}


void Editor::draw_cursor(int y_offset) const {
    int gutter_px = GUTTER_CHARS * char_width;
    int cx = gutter_px + PADDING + (cursor_col - scroll_x) * char_width;
    int cy = y_offset + (cursor_row - scroll_y) * char_height;

    if (cx >= gutter_px && cx < vp_width &&
        cy >= 0 && cy < vp_height - STATUS_BAR_HEIGHT) {
        XSetForeground(dpy.get(), gc, cursor_color.pixel);
        XFillRectangle(dpy.get(), win, gc, cx, cy, 2, char_height);
    }
}


void Editor::draw_scrollbar() const {
    if ((int)lines.size() <= 1)
        return;

    int bar_area_h = vp_height - STATUS_BAR_HEIGHT;
    int bar_w = 8;
    int bar_x = vp_width - bar_w;

    XSetForeground(dpy.get(), gc, scrollbar_bg.pixel);
    XFillRectangle(dpy.get(), win, gc, bar_x, 0, bar_w, bar_area_h);

    int vis = visible_lines_count(vp_height, char_height, STATUS_BAR_HEIGHT, PADDING);
    int total = (int)lines.size();

    double thumb_ratio = std::min(1.0, (double)vis / total);
    int thumb_h = std::max(16, (int)(bar_area_h * thumb_ratio));
    int thumb_y = (int)((double)scroll_y / total * bar_area_h);
    thumb_y = std::min(thumb_y, bar_area_h - thumb_h);

    XSetForeground(dpy.get(), gc, scrollbar_fg.pixel);
    XFillRectangle(dpy.get(), win, gc, bar_x, thumb_y, bar_w, thumb_h);
}


void Editor::draw_status_bar() const {
    int bar_y = vp_height - STATUS_BAR_HEIGHT;

    XSetForeground(dpy.get(), gc, status_bg.pixel);
    XFillRectangle(dpy.get(), win, gc, 0, bar_y, vp_width, STATUS_BAR_HEIGHT);

    char buf[256];
    snprintf(buf, sizeof(buf), " %s%s  Ln %d, Col %d  (%zu lines)  [Tab: sim | Ctrl+S: save & reload]",
             filepath_.c_str(), modified ? " *" : "",
             cursor_row + 1, cursor_col + 1, lines.size());

    XSetForeground(dpy.get(), gc, status_fg.pixel);
    XDrawString(dpy.get(), win, gc, PADDING, bar_y + char_ascent + 2, buf, strlen(buf));
}


bool Editor::is_keyword(const string &word) const {
    return word == "lut" || word == "wire" || word == "unit";
}


bool Editor::handle_key(XKeyEvent &ev) {
    char buf[32];
    KeySym ksym;
    int len = XLookupString(&ev, buf, sizeof(buf), &ksym, nullptr);

    bool ctrl = (ev.state & ControlMask) != 0;

    if (ctrl) {
        switch (ksym) {
        case XK_s: case XK_S:
            return true;   /* signal to caller: save requested */
        default:
            return false;
        }
    }

    switch (ksym) {
    case XK_Left:
        if (cursor_col > 0)
            cursor_col--;
        else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = (int)lines[cursor_row].size();
        }
        break;

    case XK_Right:
        if (cursor_col < (int)lines[cursor_row].size())
            cursor_col++;
        else if (cursor_row < (int)lines.size() - 1) {
            cursor_row++;
            cursor_col = 0;
        }
        break;

    case XK_Up:
        if (cursor_row > 0) {
            cursor_row--;
            cursor_col = std::min(cursor_col, (int)lines[cursor_row].size());
        }
        break;

    case XK_Down:
        if (cursor_row < (int)lines.size() - 1) {
            cursor_row++;
            cursor_col = std::min(cursor_col, (int)lines[cursor_row].size());
        }
        break;

    case XK_Home:
        cursor_col = 0;
        break;

    case XK_End:
        cursor_col = (int)lines[cursor_row].size();
        break;

    case XK_Return: case XK_KP_Enter: {
        string &cur = lines[cursor_row];
        string after = cur.substr(cursor_col);
        cur = cur.substr(0, cursor_col);
        lines.insert(lines.begin() + cursor_row + 1, after);
        cursor_row++;
        cursor_col = 0;
        modified = true;
        break;
    }

    case XK_BackSpace:
        if (cursor_col > 0) {
            lines[cursor_row].erase(cursor_col - 1, 1);
            cursor_col--;
            modified = true;
        } else if (cursor_row > 0) {
            cursor_col = (int)lines[cursor_row - 1].size();
            lines[cursor_row - 1] += lines[cursor_row];
            lines.erase(lines.begin() + cursor_row);
            cursor_row--;
            modified = true;
        }
        break;

    case XK_Delete:
        if (cursor_col < (int)lines[cursor_row].size()) {
            lines[cursor_row].erase(cursor_col, 1);
            modified = true;
        } else if (cursor_row < (int)lines.size() - 1) {
            lines[cursor_row] += lines[cursor_row + 1];
            lines.erase(lines.begin() + cursor_row + 1);
            modified = true;
        }
        break;

    case XK_Tab:
        return false;  /* let caller handle tab toggle */

    default:
        if (len > 0 && buf[0] >= 32 && buf[0] < 127) {
            lines[cursor_row].insert(cursor_col, 1, buf[0]);
            cursor_col++;
            modified = true;
        }
        break;
    }

    ensure_cursor_visible();
    return false;
}


void Editor::handle_button(XButtonEvent &ev) {
    int gutter_px = GUTTER_CHARS * char_width;

    switch (ev.button) {
    case 1: { /* left click — position cursor */
        if (ev.x > gutter_px) {
            int row = scroll_y + (ev.y - PADDING) / char_height;
            int col = scroll_x + (ev.x - gutter_px - PADDING) / char_width;

            row = std::max(0, std::min(row, (int)lines.size() - 1));
            col = std::max(0, std::min(col, (int)lines[row].size()));

            cursor_row = row;
            cursor_col = col;
        }
        break;
    }
    case 4: /* scroll up */
        scroll_y = std::max(0, scroll_y - 3);
        break;
    case 5: /* scroll down */
        scroll_y = std::min(std::max(0, (int)lines.size() - 1), scroll_y + 3);
        break;
    default:
        break;
    }
}


bool Editor::save() const {
    std::ofstream out(filepath_);
    if (!out.is_open())
        return false;

    for (size_t i = 0; i < lines.size(); i++) {
        out << lines[i];
        if (i < lines.size() - 1)
            out << '\n';
    }

    out << '\n';  /* trailing newline */
    return out.good();
}


string Editor::content() const {
    string result;
    for (size_t i = 0; i < lines.size(); i++) {
        result += lines[i];
        if (i < lines.size() - 1)
            result += '\n';
    }
    return result;
}
