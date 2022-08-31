#pragma once
#include <deque>
#include <string>
#include "../message_item.hpp"
#include "../buffer.hpp"
#include "../message_parser.hpp"
#include "screen_manager.hpp" 

struct message_list_screen: app_screen
{
    message_list_screen(uint64_t cur_chat_id, std::string const &cur_chat_name, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);

    void show();
    void show(size_t page_num);

    uint64_t id_chat() const { return chat_id; }

    std::string chatname() const {return chat_name;}

    void load_previous_messages();

    void handle_input(int c);

    void push_back_msg(uint64_t chat_id, uint64_t msg_id, std::string const &message);
    void push_front_msg(uint64_t chat_id, uint64_t msg_id, std::string const &message);

private:
    
    uint64_t chat_id;
    std::string chat_name;
    uint64_t first_msg_id;
    size_t page;
    size_t last_page;
    buffer buff;
};