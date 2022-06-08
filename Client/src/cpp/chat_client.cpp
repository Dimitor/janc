#include "../hpp/chat_client.hpp"

using namespace std::placeholders;

chat_client::chat_client(io_context &os_context, db_ptr chat_db, screen_manager_ptr manager, 
                         int &service_var, std::string const &username, size_t password_len)
: socket{  os_context }
, read_msg{ 580000 }
, msg_h{ chat_db, manager, service_var,  username, password_len}
{}

void chat_client::send_msg(chat_message const &msg)
{
    msg_queue.push(msg);

    if(msg_queue.size() == 1)
    {
        async_write();
    }
}

void chat_client::connect(tcp::endpoint &endpoint, std::string const &username, 
                          std::string const &password, bool is_new_user)
{
    socket.async_connect(endpoint,
        [&, is_new_user](err_code const &err)
        {
            if (!err)
            {
                msg_h.set_connection(shared_from_this());
                
                auto record = username + " " + password;
                
                chat_message msg;
                uint8_t msg_type;
                if (!is_new_user)
                {
                    msg_type = message_parser::message_types::AUTHORIZE_USER;
                }
                else
                {
                    msg_type = message_parser::message_types::REGISTER_USER;
                }
                
                msg.from_str(msg_type, record.c_str(), record.length());
                send_msg(msg);
                async_read_header();
            }
        });
}

void chat_client::async_read_header()
{
    asio::async_read(socket, asio::buffer(read_msg.data(), chat_message::header_length()), 
                     std::bind(&chat_client::on_read_header, shared_from_this(), _1, _2));
}

void chat_client::async_read_body()
{
    asio::async_read(socket, asio::buffer(read_msg.body(), read_msg.body_length()), 
                     std::bind(&chat_client::on_read_body, shared_from_this(), _1, _2));
}

void chat_client::async_write()
{
    asio::async_write(socket, asio::buffer(msg_queue.front().data(), msg_queue.front().length()), std::bind(&chat_client::on_write, shared_from_this(), _1, _2));
}

void chat_client::on_read_header(err_code err, size_t bytes_count) 
{
    if (!err && read_msg.decode_header())
    {
        async_read_body();
    }
    else
    {
        close();
    }
}

void chat_client::on_read_body(err_code err, size_t bytes_count)
{
    if (!err)
    {
        async_read_header();
        if (!(msg_h.process_message(read_msg)))
        {
            close();
        }
    }
    else 
    {
        close();
    }
}

void chat_client::on_write(err_code err, size_t bytes_count)
{
    if (!err)
    {
        msg_queue.pop();
        if (!msg_queue.empty())
        {
            async_write();
        }
    }
    else
    {
        close();
    }
}