#pragma once
#include <iostream>
#include <queue>
#include <utility>
#include <iostream>
#include <unordered_set>
#include "boost/algorithm/string.hpp"
#include "boost/asio.hpp"
#include "server_type.hpp"
#include "chat_message.hpp"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

using io_context = asio::io_context;
using socket_t = tcp::socket;
using err_code = boost::system::error_code;

using message_queue = std::queue<chat_message>;

using namespace std::placeholders;

//class for connection processing 
struct session: connection_type,
                std::enable_shared_from_this<session>
{
    session(socket_t &&srv_socket, io_context &context, server_type &serv)
    : socket{ std::move(srv_socket) }
    , reader{ context }
    , writer{ context }
    , srv   { serv }
    {}

    void start();

    void send_msg(chat_message const &msg);

    uint64_t id() const { return session_id; }

    void id(uint64_t new_id) { session_id = new_id; }

    void close() { socket.close(); }

    void close(err_code &err) { socket.close(err); }

private:

    void async_read_record_header();

    void async_read_record_body();

    void async_read_header();

    void async_read_body();

    void async_write();

    void on_read_record_header(err_code err, size_t bytes_count);

    void on_read_record_body(err_code err, size_t bytes_count);

    void on_read_header(err_code err, size_t bytes_count);

    void on_read_body(err_code err, size_t bytes_count);

    void on_write(err_code err, size_t bytes_count);

    socket_t            socket;
    io_context::strand  reader;
    io_context::strand  writer;
    chat_message        read_msg;
    message_queue       msg_queue;
    server_type         &srv;
    uint64_t            session_id;
};
