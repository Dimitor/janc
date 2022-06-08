#include "../../hpp/screens/selected_participants_screen.hpp"

selected_participants_screen::selected_participants_screen(std::vector<user_item> const &participants_ref, 
                                                           std::weak_ptr<screen_manager> scr_manager, 
                                                           scr_size_data size_data,
                                                           delete_handler_t &&delete_handler)
: app_screen(scr_manager, size_data)
, participants{participants_ref}
, delete_h{std::move(delete_handler)}
, page{0}
, cur_participant_num{0}
{title = "Selected participants";}

void selected_participants_screen::show()
{
    wclear(winparts[1]);
    
    if (participants.empty())
    {
        app_screen::show();
        return;
    }

    if (cur_participant_num >= participants.size())
    {
        cur_participant_num = participants.size() - 1;
    }
    
    size_t start = page * height;
    
    for (size_t i = start, line_num = 0; i < (page+1)*height && i < participants.size(); ++i, ++line_num)
    {
        mvwprintw(winparts[1], line_num, 1, "%s", participants[i].name.c_str());
    }

    if (cur_participant_num >= start && 
        cur_participant_num <= (page + 1) * height)
    {
        mvwprintw(winparts[1], cur_participant_num-start, 0, "*");
    }
    app_screen::show();
}

void selected_participants_screen::hide()
{
    page = 0;
    cur_participant_num = 0;
    app_screen::hide();
}

void selected_participants_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_DOWN:
        {
            if (cur_participant_num + 1 < participants.size())
            {
                ++cur_participant_num;
                if ((page + 1) * height <= cur_participant_num) { ++page; }
                print_title(true);
            }
        }
        break;
        case KEY_UP:
        {
            if (cur_participant_num > 0)
            {
                --cur_participant_num;
                if (page * height > cur_participant_num) { --page; }
                print_title(true);
            }
        }
        break;
        case KEY_DC:
        case KEY_ENTER:
        case 10:
        {
            delete_h(cur_participant_num);
        }
        break;
    }
}