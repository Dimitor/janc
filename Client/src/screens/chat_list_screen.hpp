#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include "../buffer.hpp"
#include "../scr_size_data.hpp"
#include "../chat_item.hpp"
#include "screen_manager.hpp"
#include "dialog_screen.hpp"
#include "info_screen.hpp"
#include "message_input_screen.hpp"
#include "message_list_screen.hpp"

struct chat_list_screen: app_screen
{
    chat_list_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);

    void show();
    void show(size_t page_num);

    void load_chats();

    void add_chat(uint64_t chat_id, std::string &&chat_name);

    void delete_chat(uint64_t chat_id);

    void handle_input(int c);

private:
    std::vector<std::string> lines;
    std::vector<uint64_t>    chat_ids;
    std::vector<size_t>      start_chat_line_nums;
    size_t cur_chat_num;
    size_t page;
};