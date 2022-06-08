#include "../hpp/inf_line.hpp"

inf_line::inf_line(WINDOW *line_win, size_t max_length, bool is_content_hide)
: is_hide{is_content_hide}
, maxlen{max_length}
, x{0}
, shift{0}
, maxx{getmaxx(line_win)}
, win{line_win}
{
    x = 0;
    shift = 0;
    maxx = getmaxx(line_win);
    win = line_win;
}

void inf_line::handle_input(int c)
{
    switch (c)
    {
        case KEY_LEFT:
        {
            move_left();
        }
        break;
        case KEY_RIGHT:
        {
            move_right();
        }
        break;
        case KEY_UP:
        case KEY_DOWN:
        break;
        case 127:
        case KEY_BACKSPACE:
        {
            if(x > 0) 
            { 
                --x; 
                line_.erase(x+shift, 1);
            } 
            else if (shift > 0) 
            { 
                --shift;
                line_.erase(x+shift, 1);
            }
        }
        break;
        case KEY_DC:
        {
            if (x+shift < line_.length())
            {
                line_.erase(x+shift, 1);
            } 
        }
        break;
        default:
        {
            if (line_.length() + 1 <= maxlen)
            {
                line_.insert(x+shift, 1, char(c));
                move_right();
            }
        }
        break;
    }
}

void inf_line::move_right()
{
    if (x + shift < line_.length())
    {
        if (x + 1 < maxx) 
        { 
            ++x; 
            wmove(win, 0, x);
        }
        else 
        { 
            ++shift; 
        }
    }
}

void inf_line::move_left()
{
    if (x + shift > 0)
    {
        if (x > 0) 
        { 
            --x; 
            wmove(win, 0, x);
        }
        else 
        { 
            --shift; 
        }
    }
}

void inf_line::print_line()
{
    std::string printing_subline;
    if (is_hide)
    {
        printing_subline = std::string(line_.length(), '*');
    }
    else
    {
        printing_subline = line_.substr(shift, maxx);
    }
    
    mvwprintw(win, 0, 0, "%s", printing_subline.c_str());
    wclrtoeol(win);
    wmove(win, 0, x);
    wrefresh(win);
}

void inf_line::clear()
{
    line_.clear();
    x = 0;
    shift = 0;
}