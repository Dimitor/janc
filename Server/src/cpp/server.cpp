#include "../hpp/server.hpp"

void server::async_accept() 
{ 
    acceptor.async_accept(
          [&](err_code err, socket_t socket)
          {
              if (!err)
              {
                  std::make_shared<session>(std::move(socket), context, srv)->start();
              }
              async_accept();
          }
    );
}