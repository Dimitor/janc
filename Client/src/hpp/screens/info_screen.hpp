#pragma once
#include <vector>
#include <string>
#include "screen_manager.hpp"
#include "../buffer.hpp"
#include "../scr_size_data.hpp"

struct info_screen: app_screen
{
    info_screen(uint64_t cur_chat_id, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);

    void show();
    void show(size_t page_num);

    void handle_input(int c);

    void update_participants();

private:
    
    uint64_t chat_id;
    size_t participants_quantity;
    size_t page;
    buffer buff;
};