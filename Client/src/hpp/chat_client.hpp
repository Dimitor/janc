#pragma once
#include <queue>
#include <iostream>
#include <memory>
#include <mutex>
#include "chat_message.hpp"
#include "message_handler.hpp"
#include "ncurses.h"
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

using io_context = asio::io_context;
using socket_t = tcp::socket;
using err_code = boost::system::error_code;

using message_queue = std::queue<chat_message>;

struct screen_manager;

using db_ptr = std::shared_ptr<database>;
using screen_manager_ptr = std::shared_ptr<screen_manager>;

struct chat_client: public std::enable_shared_from_this<chat_client>
                  , public connection_type
{
    chat_client(io_context &os_context, db_ptr chat_db, screen_manager_ptr manager, 
                int &service_var, std::string const &username, size_t password_len);

    void connect(tcp::endpoint &endpoint, std::string const &username, 
                 std::string const &password, bool is_new_user);

    void send_msg(chat_message const &msg);

    void close() { socket.close(); }

private:

    void async_read_header();

    void async_read_body();

    void async_write();

    void on_read_header(err_code err, size_t bytes_count);

    void on_read_body(err_code err, size_t bytes_count);

    void on_write(err_code err, size_t bytes_count);

    socket_t      socket;
    chat_message  read_msg;
    message_queue msg_queue;
    message_handler msg_h;
};