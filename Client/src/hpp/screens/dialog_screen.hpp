#pragma once
#include <functional>
#include <stdexcept>
#include "../scr_size_data.hpp"
#include "screen_manager.hpp"

using response_handler_t = std::function<void (int)>;

struct dialog_screen: app_screen
{
    dialog_screen(bool yes_no_option, std::weak_ptr<screen_manager> scr_manager, 
                  scr_size_data size_data, response_handler_t &&response_handler);

    dialog_screen(std::string const &new_message, bool yes_no_option, 
                  std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data,
                  response_handler_t &&response_handler);

    void message(std::string const &new_message) { msg = new_message; }
    void message(std::string &&new_message) { msg = std::move(new_message); }

    std::string message() { return msg; }

    void show();

    void hide();

    void handle_input(int c);
private:
    bool yes_no_opt;
    int curchoise;
    int prev_scr_num;
    std::string msg;
    response_handler_t response_h;
};