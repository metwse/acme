/**
 * @file Xeditor.hpp
 * @brief Responsive HDL text editor for X11.
 */

#ifndef XEDITOR_HPP
#define XEDITOR_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <memory>
#include <string>
#include <vector>

class Interpreter;
class Lex;


/** @brief In-window HDL text editor with syntax highlighting. */
class Editor {
public:
    Editor(std::shared_ptr<Display> dpy_, Screen *scr_, Window win_,
           const std::string &content, const std::string &filepath);

    ~Editor() = default;

    /** @brief Render the editor to the window. */
    void draw() const;

    /** @brief Handle a key press event. Returns true if content was modified. */
    bool handle_key(XKeyEvent &ev);

    /** @brief Handle a mouse button event (scrolling, clicking). */
    void handle_button(XButtonEvent &ev);

    /** @brief Update viewport dimensions on resize. */
    void set_viewport(int w, int h);

    /** @brief Save current content to the file. */
    bool save() const;

    /** @brief Get the current text content as a single string. */
    std::string content() const;

    /** @brief Get the file path. */
    const std::string &filepath() const { return filepath_; }

private:
    void ensure_cursor_visible();
    void draw_line_number(int line_num, int y) const;
    void draw_text_line(int line_idx, int y) const;
    void draw_cursor(int y_offset) const;
    void draw_scrollbar() const;
    void draw_status_bar() const;

    /** @brief Detect if a word is a keyword for syntax highlighting. */
    bool is_keyword(const std::string &word) const;

    std::shared_ptr<Display> dpy;
    Screen *scr;
    Window win;

    GC gc;
    XFontStruct *font;

    std::vector<std::string> lines;
    std::string filepath_;

    int cursor_row = 0;
    int cursor_col = 0;
    int scroll_y = 0;   /* first visible line index */
    int scroll_x = 0;   /* horizontal scroll offset in characters */

    int vp_width = 512;
    int vp_height = 384;

    int char_width = 0;
    int char_height = 0;
    int char_ascent = 0;

    static const int GUTTER_CHARS = 5;
    static const int STATUS_BAR_HEIGHT = 20;
    static const int PADDING = 4;

    XColor bg_color;
    XColor fg_color;
    XColor gutter_bg;
    XColor gutter_fg;
    XColor keyword_color;
    XColor cursor_color;
    XColor status_bg;
    XColor status_fg;
    XColor scrollbar_bg;
    XColor scrollbar_fg;

    bool modified = false;
};


#endif
