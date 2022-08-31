#pragma once
#include <vector>
#include <string>
#include <functional>
#include "../chat_item.hpp"
#include "screen_manager.hpp"
#include "../scr_size_data.hpp"

using user_item = chat_item;
using delete_handler_t = std::function<void (size_t)>;

struct selected_participants_screen: app_screen
{
    selected_participants_screen(std::vector<user_item> const &participants_ref, 
                                 std::weak_ptr<screen_manager> scr_manager, 
                                 scr_size_data size_data,
                                 delete_handler_t &&delete_handler);

    void init_handler(delete_handler_t &&delete_handler) { delete_h = std::move(delete_handler); }

    void show();

    void hide();

    void handle_input(int c);

    size_t win_height() const { return height; }

private:
    std::vector<user_item> const &participants;
    delete_handler_t delete_h;
    size_t page;
    size_t cur_participant_num;
};