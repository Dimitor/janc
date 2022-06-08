#include "../hpp/server.hpp"
#include <vector>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: server <port>";
        return 1;
    }

    try
    {
        io_context context;
        auto port = std::stoul(argv[1]);
        server serv{context, port, "jancdb"};
        serv.async_accept();

        context.run();

        std::vector<std::thread> threads;
        auto count = std::thread::hardware_concurrency() * 2;

        for (size_t n = 0; n < count; ++n)
        {
            threads.emplace_back([&]{ context.run(); });
        }

        for (auto &thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }
    catch (std::exception const &err)
    {
        std::cerr << "Error!!! " << err.what();
    }
    return 0;
}
