#include "user_list_screen.hpp"

user_list_screen::user_list_screen(std::vector<user_item> const &users_ref, 
                                   std::weak_ptr<screen_manager> scr_manager, 
                                   scr_size_data size_data,
                                   select_handler_t &&select_handler)
: app_screen(scr_manager, size_data)
, users{users_ref}
, select_h{std::move(select_handler)}
, line_num{0}
{title = "Search results";}

void user_list_screen::show()
{
    wclear(winparts[1]);

    for (size_t i = 0; i < users.size(); ++i)
    {
        mvwprintw(winparts[1], i, 1, "%s", users[i].name.c_str());
    }
    if (!users.empty())
    {
        if (line_num >= users.size())
        {
            line_num = users.size() - 1;
        }
        mvwprintw(winparts[1], line_num, 0, "*");
    }

    app_screen::show();
}

void user_list_screen::hide()
{
    line_num = 0;
    app_screen::hide();
}

void user_list_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_DOWN:
        {
            if (line_num + 1 < users.size())
            {
                ++line_num;
            }
        }
        break;
        case KEY_UP:
        {
            if (line_num > 0)
            {
                --line_num;
            }
        }
        break;
        case KEY_ENTER:
        case 10:
        {
            select_h(line_num);
        }
        break;
    }
}