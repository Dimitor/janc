#include "../../hpp/screens/create_chat_screen.hpp"

using namespace std::placeholders;

create_chat_screen::create_chat_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, found_users{}
, selected_users{}
, scr_user_list(found_users, scr_manager, 
                scr_size_data{size_data.height-2, size_data.width/2, 
                size_data.starty+2, size_data.startx},
                std::bind(&create_chat_screen::select_user, this, _1))
, scr_selected_participants(selected_users, scr_manager, 
                            scr_size_data{size_data.height-2, size_data.width/2, 
                            size_data.starty+2, size_data.startx+size_data.width/2},
                            std::bind(&create_chat_screen::delete_participant, this, _1))
, is_select_title{false}
, curscreen_num{create_chat_screen::scrnums::SEARCH_LINE}
, line_search(newwin(1, size_data.width - 3, size_data.starty + 1, size_data.startx + 2), 32)
{
    title = "Chat creating screen";
}

void create_chat_screen::select_user(size_t num)
{
    if (found_users[num].name == curusername) { return; }

    bool user_not_found = true;
    
    for (auto const &user: selected_users)
    {
        if (user.id == found_users[num].id)
        {
            user_not_found = false;
            break;
        }
    }
    if (user_not_found)
    {
        selected_users.push_back(found_users[num]);
        scr_selected_participants.print_title(false);
    }
}

void create_chat_screen::delete_participant(size_t num)
{
    if (!selected_users.empty())
    {
        selected_users.erase(selected_users.begin() + num);
        scr_selected_participants.print_title(true);
    }
}

void create_chat_screen::select_title(int response_num)
{
    if (response_num == 1)
    {
        wclear(winparts[1]);
        scr_user_list.hide();
        scr_selected_participants.hide();

        curscreen_num = scrnums::SEARCH_LINE;

        line_search.clear();
        line_search.print_line();
                    
        std::string printing_str = "Enter the name of the chat and press Enter key";
        if (selected_users.size() == 1)
        {
            printing_str += " (or just press enter to create a single chat)";
        }
        mvwprintw(winparts[1], 1, 0, "%s", printing_str.c_str());
        wrefresh(winparts[1]);

        is_select_title = true;

    }
    line_search.print_line();
}

void create_chat_screen::show()
{
    if (auto manager_p = manager.lock())
    {
        size_t height, width;
        getmaxyx(stdscr, height, width);
        scr_size_data size_data{height/2, width/2, height/4, width/4};
        manager_p->set_scr(std::make_shared<dialog_screen>(true, manager, size_data, 
                          std::bind(&create_chat_screen::select_title, this, _1)), 
                          screen_manager::scrnums::DIALOG);

        auto [username, _pswd_len] = manager_p->db()->get_user_data();
        curusername = username;
    }

    mvwprintw(winparts[1], 0, 0, ">");
    app_screen::show();
    scr_user_list.print_title();
    scr_selected_participants.print_title();
    line_search.print_line();
}

void create_chat_screen::hide()
{
    line_search.clear();
    found_users.clear();
    selected_users.clear();
    curscreen_num == scrnums::SEARCH_LINE;
    is_select_title = false;
    app_screen::hide();
    scr_user_list.hide();
    scr_selected_participants.hide();
    if (auto manager_p = manager.lock())
    {
        manager_p->scr_num(screen_manager::scrnums::CHAT_LIST);
        manager_p->scr_chat_list()->print_title();
    }
}

void create_chat_screen::update_user_list(std::vector<user_item> &&matched_users) 
{ 
    found_users = std::move(matched_users);
    show();
}

void create_chat_screen::handle_input(int c)
{
    if (auto manager_p = manager.lock(); manager_p->scr_dialog()->is_show())
    {
        manager_p->scr_dialog()->handle_input(c);
        return;
    }
    if (is_select_title)
    {
        std::string chat_name = line_search.line();
        boost::algorithm::trim(chat_name);

        if (c == 10 || c == KEY_ENTER)
        {
            message_parser::parsed_message message_info;
            if ((selected_users.size() == 1) && (chat_name.length() == 0))
            {
                message_info.type = message_parser::message_types::CREATE_SINGLE_CHAT;
                message_info.idargs = {selected_users[0].id};

                chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

                if (auto manager_p = manager.lock())
                {
                    manager_p->conn()->send_msg(sending_msg);
                }

                hide();
            } 
            else if (chat_name.length() > 0)
            {
                message_info.type = message_parser::message_types::CREATE_GROUP;
                message_info.idargs = {selected_users.size()};

                for (size_t i = 0; i < selected_users.size(); ++i)
                {
                    message_info.idargs.push_back(selected_users[i].id);
                }
                message_info.strargs = {chat_name};

                chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

                if (auto manager_p = manager.lock())
                {
                    manager_p->conn()->send_msg(sending_msg);
                }

                hide();
            }
        }
        else
        {
            line_search.handle_input(c);
            line_search.print_line();
        }
        
        return;
    }
    switch (c)
    {
        case KEY_BTAB: 
        case KEY_CTAB: 
        case KEY_STAB: 
        case KEY_CATAB: 
        case 9:
        {
            switch (curscreen_num)
            {
                case scrnums::SEARCH_LINE:
                {
                    mvwprintw(winparts[1], 0, 0, "");
                    curscreen_num = scrnums::SEARCH_RESULTS;
                    scr_user_list.print_title(true);
                    scr_selected_participants.print_title(false);
                }
                break;
                case scrnums::SEARCH_RESULTS:
                {
                    mvwprintw(winparts[1], 0, 0, "");
                    curscreen_num = scrnums::SELECTED_ITEMS;
                    scr_user_list.print_title(false);
                    scr_selected_participants.print_title(true);
                }
                break;
                case scrnums::SELECTED_ITEMS:
                {
                    mvwprintw(winparts[1], 0, 0, ">");
                    curscreen_num = scrnums::SEARCH_LINE;
                    scr_user_list.print_title(false);
                    scr_selected_participants.print_title(false);
                    line_search.print_line();
                }
                break;
            } 
        }
        break;
        default:
        switch (curscreen_num)
        {
            case scrnums::SEARCH_LINE:
            {
                if (c == 10 || c == KEY_ENTER)
                {
                    if (selected_users.empty()) { return; }
                    if (auto manager_p = manager.lock())
                    {
                        ((dialog_screen*)manager_p->scr_dialog().get())->message("Do you want to finish adding participants?");
                        manager_p->scr_dialog()->show();
                    }
                }
                else
                {
                    line_search.handle_input(c);
                    line_search.print_line();
                    if (line_search.line_len() == 0)
                    {
                        found_users.clear();
                        show();
                        return;
                    }
                    if (auto manager_p = manager.lock())
                    {
                        message_parser::parsed_message message_info;
                        message_info.type    = message_parser::message_types::SEARCH_USERS;
                        message_info.idargs  = {scr_user_list.win_height()};
                        message_info.strargs = {line_search.line()};

                        chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

                        manager_p->conn()->send_msg(sending_msg);
                    }
                }
            }
            break;
            case scrnums::SEARCH_RESULTS:
            {
                scr_user_list.init_handler(std::bind(&create_chat_screen::select_user, this, _1));
                scr_user_list.handle_input(c);
                scr_user_list.print_title(true);
            }
            break;
            case scrnums::SELECTED_ITEMS:
            {
                scr_selected_participants.init_handler(std::bind(&create_chat_screen::delete_participant, this, _1));
                scr_selected_participants.handle_input(c);
                scr_selected_participants.print_title(true);
            }
            break;
        }
    }
}