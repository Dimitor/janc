#pragma once
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ncurses.h>
#include "buffer.hpp"

struct editor
{
    editor(WINDOW *edtwin, size_t max_symbols_count = -1);

    int handle_input(int c);
    
    std::string text();
    
    void print_buff();

    void clear();

    void refresh_cursor() { wmove(win, y, x); }

private:
    WINDOW *win;
    int x, y, maxy, maxx, shift;
    size_t symbols_count, max_symbols_quantity;
    std::unique_ptr<buffer> buff;

    void move_up();
    void move_down();
    void move_left();
    void move_right();

    void delete_line();                  
    void delete_line(int);
};