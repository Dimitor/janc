#include "chat_list_screen.hpp"

chat_list_screen::chat_list_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, chat_ids{}
, start_chat_line_nums{}
, cur_chat_num{0}
, page{0}
{title = "Chat list";}

void chat_list_screen::show()
{
    wclear(winparts[1]);
    size_t start = page * height;
    for (size_t i = start, line_num = 0; i < lines.size() && i < (page + 1) * height; ++i, ++line_num)
    {
        mvwprintw(winparts[1], line_num, 1, "%s", lines[i].c_str());
        wclrtoeol(winparts[1]);
    }
    if ((!lines.empty()) && (start_chat_line_nums[cur_chat_num] >= start)
        && (start_chat_line_nums[cur_chat_num] < (page + 1) * height))
    {
        for (size_t i = start_chat_line_nums[cur_chat_num], 
                    line_num = start_chat_line_nums[cur_chat_num] - start; 
                    i < (page + 1) * height && lines[i] != "\n"; 
                    ++i, ++line_num)
        {
            mvwprintw(winparts[1], line_num, 0, "*");
            mvwprintw(winparts[1], line_num, 1, "%s", lines[i].c_str());
            wclrtoeol(winparts[1]);
        }
    }

    if (auto manager_p = manager.lock())
    {
        size_t height, width;
        getmaxyx(stdscr, height, width);
        scr_size_data size_data{height/2, width/2, height/4, width/4};
        manager_p->set_scr(std::make_shared<dialog_screen>(false, manager, size_data, [](int){}), 
        screen_manager::scrnums::DIALOG);
        ((dialog_screen*)manager_p->scr_dialog().get())->message("CTRL+P - profile data screen\nCTRL+I - chat info (on chat screen)\nCTRL+N - create new chat\nCTRL+L - return to chat list screen");
    }

    mvwprintw(winparts[0], height+1, 1, "shortcuts hint: h");
    app_screen::show();
}

void chat_list_screen::show(size_t page_num)
{
    page = page_num;
    show();
}

void chat_list_screen::load_chats()
{
    if (auto manager_p = manager.lock())
    {
        auto chats = manager_p->db()->get_chats();

        for (auto &chat: chats)
        {
            add_chat(chat.id, std::move(chat.name));
        }
    }
}

void chat_list_screen::add_chat(uint64_t chat_id, std::string &&chat_name)
{
    start_chat_line_nums.push_back(lines.size());
    std::string chatname = std::move(chat_name);
    size_t lines_count = chatname.length() / (width-1);
    for (size_t i = 0; i < lines_count; ++i)
    {
        lines.push_back(chatname.substr(i*(width-1), (width-1)));
    }
    
    auto last_str_len = chatname.length() % (width-1);
    std::string last_str = chatname.substr(chatname.length() - last_str_len, last_str_len);

    if (!last_str.empty())
    {
        lines.push_back(last_str);
    }

    lines.push_back("\n");
    chat_ids.push_back(chat_id);
}

void chat_list_screen::delete_chat(uint64_t chat_id)
{
}

void chat_list_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_DOWN:
        {
            if (cur_chat_num + 1 < chat_ids.size())
            {
                ++cur_chat_num;
               
                if (start_chat_line_nums[cur_chat_num] >= (page + 1) * height)
                {
                    ++page;
                }
                print_title(true);
            }
        }
        break;
        case KEY_UP:
        {
            if (cur_chat_num > 0)
            {
                --cur_chat_num;
               
                if (start_chat_line_nums[cur_chat_num] < page * height)
                {
                    --page;
                }
                print_title(true);
            }
        }
        break;
        case KEY_RIGHT:
        {
            if ((page + 1) * height < lines.size())
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
        case KEY_ENTER:
        case 10:
        {
            if (auto manager_p = manager.lock())
            {
                wclear(winparts[0]);
                wrefresh(winparts[0]);
                auto chat_id = chat_ids[cur_chat_num];
                manager_p->scr_num(screen_manager::scrnums::MESSAGE_INPUT);
                if (manager_p->scr_message_list() && 
                    ((message_list_screen*)manager_p->scr_message_list().get())->id_chat() == chat_id)
                {
                    manager_p->scr_message_list()->print_title(false);
                    manager_p->scr_message_input()->print_title(true);
                }
                else
                {
                    std::string chat_name = manager_p->db()->get_chat_name(chat_id);
                    size_t win_height, win_width;
                    getmaxyx(stdscr, win_height, win_width);
                    scr_size_data size_data{ win_height/2, win_width, 0, 0 };
                    manager_p->set_scr(std::make_shared<message_list_screen>(chat_id, chat_name, manager, size_data), 
                                       screen_manager::scrnums::MESSAGE_LIST);
                    manager_p->scr_message_list()->print_title(false);

                    size_data = {win_height/2, win_width, win_height/2, 0};
                    manager_p->set_scr(std::make_shared<message_input_screen>(chat_id, manager, size_data), 
                                       screen_manager::scrnums::MESSAGE_INPUT);
                    manager_p->scr_message_input()->print_title(true);

                    size_data = {win_height, win_width/3, 0, win_width/3 * 2};
                    manager_p->set_scr(std::make_shared<info_screen>(chat_id, manager, size_data), 
                                       screen_manager::scrnums::CHAT_INFO);
                    ((info_screen*)manager_p->scr_chat_info().get())->update_participants();
                    manager_p->scr_chat_info()->hide();
                }
            }
        }
        break;
    }
}