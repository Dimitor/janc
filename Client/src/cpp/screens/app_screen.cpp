#include "../../hpp/screens/app_screen.hpp"

app_screen::app_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
{
    showed = false;
    height        = size_data.height;
    width         = size_data.width;
    size_t starty = size_data.starty;
    size_t startx = size_data.startx;

    winparts[0] = newwin(height, width, starty, startx);
    box(winparts[0], 0, 0);
    winparts[1] = newwin(height - 2, width - 2, starty + 1, startx + 1);

    keypad(winparts[0], true);

    panels[0] = new_panel(winparts[0]);
    panels[1] = new_panel(winparts[1]);

    keypad(winparts[1], true);
    
    height -= 2;
    width  -= 2;

    manager = scr_manager;
}

void app_screen::print_title(std::string const &new_title, bool is_selected)
{
    title = new_title;
    print_title(is_selected);
}

void app_screen::print_title(bool is_selected)
{
    box(winparts[0], 0, 0);
    std::string title_;
    if (is_selected)
    {
        title_ = ">" + title + "<";
    }
    else
    {
        title_ = title;
    }

    size_t len = title_.length() <= (width-2) ? title_.length() : width - 2;

    std::string printing_str(title_.c_str(), len);

    mvwprintw(winparts[0], 0, (width+2)/2 - len/2, "%s", printing_str.c_str());

    show();
}

void app_screen::show()
{
    show_panel(panels[0]);
    show_panel(panels[1]);
    update_panels();
    doupdate();
    showed = true;
}

void app_screen::hide()
{
    hide_panel(panels[0]);
    hide_panel(panels[1]);
    update_panels();
    doupdate();
    showed = false;
}