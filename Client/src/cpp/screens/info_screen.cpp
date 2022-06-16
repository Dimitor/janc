#include <vector>
#include "../../hpp/screens/info_screen.hpp"

info_screen::info_screen(uint64_t cur_chat_id, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, chat_id{cur_chat_id}
, page{0}
, buff(width)
{title = "Chat info";}

void info_screen::show()
{
    wclear(winparts[1]);
    mvwprintw(winparts[0], height + 1, 1, "%lu users", participants_quantity);

    for (size_t i = page * height, scr_line_num = 0; i < buff.lines.size() && scr_line_num < height; ++i, ++scr_line_num)
    {
        mvwprintw(winparts[1], scr_line_num, 0, "%s", buff.lines[i].c_str());
        wclrtoeol(winparts[1]);
    }
    app_screen::show();
}

void info_screen::show(size_t page_num)
{
    page = page_num;
    show();
}

void info_screen::update_participants()
{
    participants_quantity = 0;
    std::vector<std::string> participants;
    if (auto manager_p = manager.lock())
    { 
        participants = manager_p->db()->get_chat_participants(chat_id);
    }

    buff.clear();

    participants_quantity = participants.size();

    for (auto &participant_name: participants)
    {
        buff.append_line(participant_name);
    }
}

void info_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_RIGHT:
        {
            if ((page + 1) * height < buff.lines.size())
            {
                ++page;
                print_title(true);
            }
        }
        break;
        case KEY_LEFT:
        {
            if (page > 0)
            {
                --page;
                print_title(true);
            }
        }
        break;
    }
}