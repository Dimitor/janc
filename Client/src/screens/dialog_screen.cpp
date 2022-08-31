#include "dialog_screen.hpp"

dialog_screen::dialog_screen(bool yes_no_option, std::weak_ptr<screen_manager> scr_manager, 
                             scr_size_data size_data, response_handler_t &&response_handler)
: app_screen(scr_manager, size_data)
, yes_no_opt{yes_no_option}
, curchoise{static_cast<int>(yes_no_opt)}
, response_h{std::move(response_handler)}
{
    if (auto manager_p = scr_manager.lock())
    {
        prev_scr_num = manager_p->scr_num();
    }
}

dialog_screen::dialog_screen(std::string const &new_message, bool yes_no_option,
                             std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data,
                             response_handler_t &&response_handler)
: app_screen(scr_manager, size_data)
, yes_no_opt{yes_no_option}
, curchoise{static_cast<int>(yes_no_opt)}
, msg{new_message}
, response_h{std::move(response_handler)}
{
    if ((size_data.width)*(size_data.height-1) < new_message.length())
    {
        throw std::logic_error("Message is too long!");
    }

    if (auto manager_p = scr_manager.lock())
    {
        prev_scr_num = manager_p->scr_num();
    }
}

void dialog_screen::show()
{
    if (auto manager_p = manager.lock())
    {
        manager_p->scr_num(screen_manager::scrnums::DIALOG);
    }
    
    wclear(winparts[1]);
    if (yes_no_opt)
    {
        mvwprintw(winparts[1], height-1, width/4-1, "YES");
        mvwprintw(winparts[1], height-1, width/4*3-1, "NO");

        if (curchoise == 1)
        {
            mvwprintw(winparts[1], height-1, width/4-2, ">");
            mvwprintw(winparts[1], height-1, width/4+2, "<");
            mvwprintw(winparts[1], height-1, width/4*3-2, " ");
            mvwprintw(winparts[1], height-1, width/4*3+1, " ");
        }
        else
        {
            mvwprintw(winparts[1], height-1, width/4-2, " ");
            mvwprintw(winparts[1], height-1, width/4+2, " ");
            mvwprintw(winparts[1], height-1, width/4*3-2, ">");
            mvwprintw(winparts[1], height-1, width/4*3+1, "<");
        }
    }
    else
    {
        mvwprintw(winparts[1], height-1, width/2-2, ">OK<");
    }

    mvwprintw(winparts[1], 0, 0, "%s", msg.c_str());

    app_screen::show();
}

void dialog_screen::hide()
{
    if (auto manager_p = manager.lock())
    {
        manager_p->scr_num(prev_scr_num);
    }

    app_screen::hide();
}

void dialog_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_RIGHT:
        case KEY_LEFT:
        {
            curchoise = -curchoise;
            show();
        }
        break;
        case KEY_ENTER:
        case 10:
        {
            hide();
            response_h(curchoise);
        }
    }
}