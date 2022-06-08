#include "../hpp/editor.hpp"

editor::editor(WINDOW *edtwin, size_t max_symbols_count)
{
    x = 0;
    y = 0;
    shift = 0;
    symbols_count=0;
    max_symbols_quantity = max_symbols_count;
    getmaxyx(edtwin, maxy, maxx);

    buff = std::make_unique<buffer>(maxx);
    buff->append_line("");
    win = edtwin;
}

int editor::handle_input(int c)
{
    switch(c)
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
        {
            move_up();
        }
        break;
        case KEY_DOWN:
        {
            move_down();
        }
        break;
        case 127:
        case KEY_BACKSPACE:
        {
            if(x > 0)
            {
                buff->delete_char(y+shift, --x);    
                if (symbols_count != 0) { symbols_count--; }
            }
            else if (y + shift > 0)
            {
                    x = buff->lines[y-1+shift].length();
                    move_up();
                    buff->delete_char(y+shift, --x);    
                    if (symbols_count != 0) { symbols_count--; }
            }
        }
        break;
        case KEY_DC:
        {
            if (x < buff->lines[y+shift].length())
            {
                buff->delete_char(y+shift, x);
                
                if (symbols_count != 0) { symbols_count--; }
        
                if (buff->lines[y + shift].empty()) 
                {
                    delete_line(); 
                    move_left(); 
                }
            } 
        }
        break;
        default:
        {
            if (symbols_count == max_symbols_quantity) { break; }
            buff->insert_char(char(c), y+shift, x);
            move_right();
            
            if (x == 0) { move_right(); }
            
            symbols_count++;
        }
        break;
    }
    return symbols_count;
}

void editor::move_left()
{
    if(x > 0)
    {
        x--;
    }
    else if (y > 0 || shift > 0)
    {
        move_up();
        x = buff->lines[y+shift].length();
    }
    wmove(win, y, x);
}

void editor::move_right()
{
    if(x+1 <= buff->lines[y+shift].length())
    {
        x++;
    } 
    else if (y+shift < buff->lines.size() - 1)
    {
        move_down();
        x = 0;
    }
    wmove(win, y, x);
}

void editor::move_up()
{
    if(y > 0)
    {
        y--;
    }
    else if (shift > 0)
    {
        shift--;
    }
    if(x >= buff->lines[y+shift].length())
    {
        x = buff->lines[y+shift].length();
    }
    
    wmove(win, y, x);
}

void editor::move_down()
{
    if (y+shift+1 < buff->lines.size())
    {
        if (y+1 < maxy)
        {
            y++;
        }
        else
        {
            shift++;
        }
    }
    if(x >= buff->lines[y + shift].length())
    {
        x = buff->lines[y + shift].length();
    }
    
    wmove(win, y, x);
}

void editor::print_buff()
{
    for(int i = 0; i < maxy; i++)
    {
        if(i+shift >= buff->lines.size())
        {
            wmove(win, i, 0);
        }
        else
        {
            mvwprintw(win, i, 0, buff->lines[i + shift].c_str());
        }
        wclrtoeol(win);
    }
    wmove(win, y, x);
}

void editor::delete_line()
{
    buff->remove_line(y+shift);
}

void editor::delete_line(int i)
{
    buff->remove_line(i);
}

std::string editor::text()
{
    std::string txt;
    txt.reserve(buff->lines.size() * maxx);

    for (auto &line: buff->lines)
    {
        txt += line;
    }

    return txt;
}

void editor::clear()
{
    buff->clear();
    x = 0;
    y = 0;
    shift = 0;
    symbols_count = 0;
}