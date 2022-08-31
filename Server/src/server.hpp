#pragma once
#include <optional>
#include <iostream>
#include <vector>
#include "boost/asio.hpp"
#include "session.hpp"
#include "database.hpp"

//class for accepting and managing connections
struct server
{
    server(io_context &os_context, unsigned long port, std::string &&dbname)
    : context { os_context }
    , acceptor( os_context, tcp::endpoint(tcp::v4(), port) )
    , db      { std::move(dbname) }
    , srv     { db }
    {}

    void async_accept();


private:

    io_context    &context;
    tcp::acceptor acceptor;
    database      db;
    server_type   srv;
};