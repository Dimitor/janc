#pragma once
#include <string>
#include "../message_parser.hpp"
#include "../inf_line.hpp"
#include "../scr_size_data.hpp"
#include "dialog_screen.hpp"
#include "screen_manager.hpp"

struct profile_screen: app_screen
{
    profile_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);
    
    void show();

    void hide();

    void handle_input(int c);

    void init_user_data();

    void change_username(std::string const &new_username);
    void change_password_len(size_t new_len);

private:

    bool is_allowed_chars(std::string const &str);

    void dialog_responce_handler(int);

    WINDOW *win_username;
    WINDOW *win_password_len;
    PANEL  *panel_username;
    PANEL  *panel_password_len;
    inf_line line_username;
    inf_line line_password;
    int curline;
    bool is_changing;
    std::string username;
    size_t password_len;
};