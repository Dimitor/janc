#include "../hpp/session.hpp"

void session::start()
{
    async_read_record_header();
}

void session::send_msg(chat_message const &msg)
{
    bool write_in_progress = !msg_queue.empty();
    msg_queue.push(msg);
    
    if(!write_in_progress)
    {
        async_write();
    }
}

void session::async_read_record_header()
{
    auto record_header_read_handler = std::bind(&session::on_read_record_header, 
                                                shared_from_this(), _1, _2);
    asio::async_read(socket, asio::buffer(read_msg.data(), chat_message::header_length()), 
                    asio::bind_executor(reader, record_header_read_handler));
}

void session::async_read_record_body()
{
    auto record_body_read_handler = std::bind(&session::on_read_record_body, 
                                              shared_from_this(), _1, _2);
    asio::async_read(socket, asio::buffer(read_msg.body(), read_msg.body_length()), 
                     asio::bind_executor(reader, record_body_read_handler));
}

void session::async_read_header()
{
    auto header_read_handler = std::bind(&session::on_read_header, shared_from_this(), _1, _2);
    asio::async_read(socket, asio::buffer(read_msg.data(), chat_message::header_length()), 
                     asio::bind_executor(reader, header_read_handler));
}

void session::async_read_body()
{
    auto body_read_handler = std::bind(&session::on_read_body, shared_from_this(), _1, _2);
    asio::async_read(socket, asio::buffer(read_msg.body(), read_msg.body_length()), 
                     asio::bind_executor(reader, body_read_handler));
}

void session::async_write()
{
    auto write_handler = std::bind(&session::on_write, shared_from_this(), _1, _2);
    asio::async_write(socket, asio::buffer(msg_queue.front().data(), msg_queue.front().length()), 
                      asio::bind_executor(writer, write_handler));
}

void session::on_read_record_header(err_code err, size_t bytes_count)
{
    if (!err && read_msg.decode_header())
    {
        async_read_record_body();
    }
    else
    {
       close(err);
    }
}

void session::on_read_record_body(err_code err, size_t bytes_count)
{
    if (!err && srv.process_record(read_msg, shared_from_this()))
    {
        async_read_header();
    }
    else
    {
        close(err);
    }
}

void session::on_read_header(err_code err, size_t bytes_count)
{
    if (!err && read_msg.decode_header())
    {
        async_read_body();
    }
    else
    {
        srv.leave(session_id, shared_from_this());
        close(err);
    }
}

void session::on_read_body(err_code err, size_t bytes_count)
{
    if (!err)
    {
        async_read_header();
        if (!srv.process_message(read_msg, shared_from_this()))
        {
            close(err);
            srv.leave(session_id, shared_from_this());
        }
    }
    else
    {
        srv.leave(session_id, shared_from_this());
        close(err);
    }
}

void session::on_write(err_code err, size_t bytes_count)
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
        srv.leave(session_id, shared_from_this());
        close(err);
    }
}
