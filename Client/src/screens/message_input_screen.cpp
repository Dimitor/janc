#include "message_input_screen.hpp"

message_input_screen::message_input_screen(uint64_t cur_chat_id, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, chat_id{cur_chat_id}
, ed(winparts[1])
{title = "Message text";}

void message_input_screen::handle_input(int c)
{
    if (c == 10 || c == KEY_ENTER)
    {
        std::string message_text = ed.text();

        if (auto manager_ptr = manager.lock(); !message_text.empty())
        {
            message_parser::parsed_message message_info{message_parser::message_types::MESSAGE, 
                                                        std::vector{chat_id}, 
                                                        std::vector{message_text}};
            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);
            manager_ptr->conn()->send_msg(sending_msg);
        }
        ed.clear();
        ed.print_buff();
        print_title(true);
        
        return;
    }
    
    symbols_count = ed.handle_input(c);
    ed.print_buff();
    print_title(true);
    if (symbols_count != 0)
    {
        mvwprintw(winparts[0], height + 1, 1, "Symbols count: %zu", symbols_count);
    }
    wrefresh(winparts[0]);
    wrefresh(winparts[1]);
}

void message_input_screen::hide()
{
    ed.clear();
    app_screen::hide();
}