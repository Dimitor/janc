#pragma once
#include <boost/algorithm/string.hpp>
#include "../editor.hpp"
#include "../message_parser.hpp"
#include "../chat_message.hpp"
#include "../scr_size_data.hpp"
#include "screen_manager.hpp"

struct message_input_screen: app_screen
{
    message_input_screen(uint64_t cur_chat_id, std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);

    void handle_input(int c);

    void hide();

private:
    uint64_t chat_id;
    size_t symbols_count;
    editor ed;
};