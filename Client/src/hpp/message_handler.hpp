#pragma once
#include <memory>
#include <mutex>
#include "chat_message.hpp"
#include "connection_type.hpp"
#include "database.hpp"
#include "chat_item.hpp"
#include "screens/screen_manager.hpp"
#include "screens/app_screen.hpp"
#include "screens/chat_list_screen.hpp"
#include "screens/create_chat_screen.hpp"
#include "screens/info_screen.hpp"
#include "screens/message_list_screen.hpp"
#include "screens/message_input_screen.hpp"
#include "screens/profile_screen.hpp"

using connection_ptr     = std::weak_ptr<connection_type>;
using db_ptr             = std::shared_ptr<database>;
using screen_manager_ptr = std::shared_ptr<screen_manager>;

struct message_handler
{
    message_handler(db_ptr handler_db, screen_manager_ptr handler_manager, 
                    int &service_variable, std::string const &user_name, size_t password_length)
    : db{handler_db}
    , manager{handler_manager}
    , service_var{service_variable}
    , username{user_name}
    , password_len{password_length}
    {}

    void set_connection(connection_ptr new_conn) { conn = new_conn; }
    
    bool process_message(chat_message const &msg);

private:
    db_ptr db;
    screen_manager_ptr manager;
    connection_ptr conn;
    std::mutex io_mutex;
    int &service_var;
    std::string const &username;
    size_t password_len;
};