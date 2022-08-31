#pragma once
#include <vector>
#include <string>
#include <boost/algorithm/string/trim.hpp>
#include "user_list_screen.hpp"
#include "selected_participants_screen.hpp"
#include "dialog_screen.hpp"
#include "../scr_size_data.hpp"
#include "../inf_line.hpp"
#include "../chat_message.hpp"
#include "../message_parser.hpp"
#include "screen_manager.hpp"

struct create_chat_screen: app_screen
{
    create_chat_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);

    void select_user(size_t num);

    void delete_participant(size_t num);

    void select_title(int response_num);

    void show();

    void hide();

    void update_user_list(std::vector<user_item> &&matched_users);

    void handle_input(int c);

private:

    enum scrnums
    {
        SEARCH_LINE    = 0,
        SEARCH_RESULTS = 1,
        SELECTED_ITEMS = 2
    };

    std::vector<user_item> found_users;
    std::vector<user_item> selected_users;

    user_list_screen             scr_user_list;
    selected_participants_screen scr_selected_participants;

    bool is_select_title;
    size_t curscreen_num;
    std::string curusername;
    
    inf_line line_search;
};