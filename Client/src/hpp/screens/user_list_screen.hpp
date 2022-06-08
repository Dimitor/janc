#pragma once
#include <vector>
#include <string>
#include <functional>
#include "screen_manager.hpp"
#include "../chat_item.hpp"
#include "../scr_size_data.hpp"

using user_item = chat_item;
using select_handler_t = std::function<void (size_t)>;

struct user_list_screen: app_screen
{
    user_list_screen(std::vector<user_item> const &users_ref, 
                     std::weak_ptr<screen_manager> scr_manager, 
                     scr_size_data size_data,
                     select_handler_t &&select_handler);

    void init_handler(select_handler_t &&select_handler) { select_h = std::move(select_handler); }

    void show();

    void hide();

    void handle_input(int c);

    size_t win_height() const { return height; }

private:
    std::vector<user_item> const &users;
    select_handler_t select_h;
    size_t line_num;
};