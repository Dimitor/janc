#pragma once
#include <string>
#include <iostream>
#include <limits>
#include "ncurses.h"

struct inf_line
{
    inf_line(WINDOW *line_win, size_t max_length = std::numeric_limits<size_t>::max()-1, bool is_content_hide = false);
    
    void handle_input(int c);

    void print_line();

    void line(std::string const &new_line) { line_ = new_line; x=shift=0;}
    void line(std::string &&new_line)      { line_ = std::move(new_line); x=shift=0; }

    std::string line() const { return line_; }

    size_t line_len() const { return line_.length(); }

    void max_length(size_t new_maxlen) { maxlen = new_maxlen; }
    size_t max_length() const { return maxlen; }

    void clear();

private:
    void move_right();
    void move_left();

    bool is_hide;
    size_t maxlen;
    int x, shift, maxx;
    WINDOW *win;
    std::string line_;
};