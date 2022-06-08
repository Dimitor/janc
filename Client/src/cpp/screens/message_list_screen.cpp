#include "../../hpp/screens/message_list_screen.hpp"

message_list_screen::message_list_screen(uint64_t cur_chat_id, std::string const &cur_chat_name, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, chat_id{cur_chat_id}
, chat_name{cur_chat_name}
, first_msg_id{1000000000000000}
, page{0}
, last_page{0}
, buff(width)
{
    title = cur_chat_name;
    if (auto manager_p = manager.lock())
    {
        std::vector<message_item> messages = manager_p->db()->get_messages(chat_id, 0, 1000000000000000);
        for (message_item &msg: messages)
        {
            std::string prefix = manager_p->db()->get_message_prefix(msg.id);
            buff.append_line(prefix + msg.content);
            buff.append_line("\n");
        }
    }
}

void message_list_screen::show()
{ 
    if ((last_page + 1) * height < buff.lines.size())
    {
        size_t new_last_page = buff.lines.size()/height;
        if (page == last_page) { page = new_last_page; }
        
        last_page = new_last_page;
    }

    wclear(winparts[1]);

    for (size_t i = page * height, scr_line_num = 0; i < buff.lines.size() && i < (page + 1) * height; ++i, ++scr_line_num)
    {
        mvwprintw(winparts[1], scr_line_num, 0, "%s", buff.lines[i].c_str());
        wclrtoeol(winparts[1]);
    }
    app_screen::show();
}

void message_list_screen::show(size_t page_num)
{
    page = page_num;
    show();
}

void message_list_screen::push_back_msg(uint64_t chat_id, uint64_t msg_id, std::string const &message)
{
    if (this->chat_id == chat_id)
    {
        buff.append_line(message);
        buff.append_line("\n");

        show();
    }
}

void message_list_screen::push_front_msg(uint64_t chat_id, uint64_t msg_id, std::string const &message)
{
    if (this->chat_id == chat_id)
    {
        size_t lines_count = message.length() / width;
        std::vector<std::string> ins_lines;
        for (size_t i = 0; i < lines_count; ++i)
        {
            ins_lines.push_back(message.substr(i*width, width));
        }

        auto last_str_len = message.length() % width;
        std::string last_str = message.substr(message.length() - last_str_len, last_str_len);

        if (!last_str.empty())
        {
            ins_lines.push_back(last_str);
        }

        ins_lines.push_back("\n");

        for (auto it = ins_lines.rbegin(); it != ins_lines.rend(); ++it)
        {
            buff.insert_line(*it, 0);
        }
        first_msg_id = msg_id;

        show();
    }
}

void message_list_screen::load_previous_messages()
{
    message_parser::parsed_message msg_data;
    msg_data.type = message_parser::message_types::LOAD_MESSAGES;
    msg_data.idargs = {chat_id, first_msg_id, height};

    chat_message sending_msg = message_parser::parsed_to_chat_message(msg_data);

    if (auto manager_ptr = manager.lock())
    {
        manager_ptr->conn()->send_msg(sending_msg);
    }
}

void message_list_screen::handle_input(int c)
{
    switch (c)
    {
        case KEY_RIGHT:
        {
            if (page < last_page)
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