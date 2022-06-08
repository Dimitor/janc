#include <string>
#include <thread>
#include <locale>
#include <memory>
#include <chrono>
#include "panel.h"
#include "boost/asio.hpp"
#include "../hpp/database.hpp"
#include "../hpp/chat_client.hpp"
#include "../hpp/screens/screen_manager.hpp"
#include "../hpp/scr_size_data.hpp"

std::string enter_str(std::string const &enter_msg)
{
    printw("%s: ", enter_msg.c_str());
    clrtoeol();
    char str[256];
    scanw("%s", str);

    return std::string(str);
}

bool is_allowed_char(std::string const &str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { 
    unsigned char char_num = static_cast<unsigned char>(c);
    return (char_num == 95) || ((char_num > 47) && (char_num < 58)) || 
          ((char_num > 64) && (char_num < 91)) || ((char_num > 96) && (char_num < 123));
    });
}

void init_manager(std::shared_ptr<screen_manager> manager)
{
    size_t height, width;
    getmaxyx(stdscr, height, width);

    scr_size_data size_data{ height, width, 0, 0 };

    manager->set_scr(std::make_shared<chat_list_screen>(manager, size_data), screen_manager::scrnums::CHAT_LIST);
    ((chat_list_screen*)manager->scr_chat_list().get())->load_chats();

    manager->set_scr(std::make_shared<create_chat_screen>(manager, size_data), screen_manager::scrnums::CREATE_CHAT);

    size_data = { height/2, width - 4, height/2, 1};

    manager->set_scr(std::make_shared<profile_screen>(manager, size_data), screen_manager::scrnums::PROFILE_INFO);
    
    manager->scr_num(screen_manager::scrnums::CHAT_LIST);
}

int main(int argc, char *argv[])
{
    if ((argc != 3 && argc != 4) || (argc == 4 && strcmp(argv[3], "-r") != 0 && strcmp(argv[3], "-a") != 0))
    {
        std::cerr << "Usage: junc <ip> <port> [-r | -a]" << std::endl;
        return 1;
    }

    try
    {
        initscr();
        raw();
        keypad(stdscr, true);

        std::string ip{argv[1]};
        std::string port{argv[2]};
        std::string username;
        std::string password;
        bool reg_flag = false;

        auto db = std::make_shared<database>("janc");

        if (argc == 3)
        {
            if (db->user_exists())
            {
                db->clear_db(false);
                username = std::get<0>(db->get_user_data());
            }
            else
            {
                db->clear_db();
                username = enter_str("Enter username");
                if (!is_allowed_char(username))
                {
                    std::cerr << "Only the characters a-z, A-Z, 0-9 and _ are available!" << std::endl;
                    exit(2);
                }
                else if (username.length() > 32)
                {
                    std::cerr << "Username must not be longer than 32 characters!" << std::endl;
                    exit(3);
                }
            }
            noecho();
            password = enter_str("Enter password");
        }
        else
        {
            db->clear_db();
            reg_flag = strcmp(argv[3], "-r") == 0;

            username = enter_str("Enter username");

            if (!is_allowed_char(username))
            {
                std::cerr << "Only the characters a-z, A-Z, 0-9 and _ are available!" << std::endl;
                exit(2);
            }
            else if (username.length() > 32)
            {
                std::cerr << "Username must not be longer than 32 characters!" << std::endl;
                exit(3);
            }

            noecho();
            password = enter_str("Enter password");
            
            if (reg_flag)
            {
                std::string confirm_password = enter_str("Confirm password");

                if (confirm_password != password)
                {
                    std::cerr << "Passwords don't match!" << std::endl;
                    exit(4);
                }
            }
        }

        int service_var = 0;

        auto manager = std::make_shared<screen_manager>(db);
        
        init_manager(manager);

        io_context context;
        tcp::endpoint endpoint{asio::ip::make_address(ip), (unsigned short)std::stoul(port)};

        chat_client user{context, db, manager, service_var, username, password.length()};

        std::shared_ptr<chat_client> user_ptr{&user};

        std::thread network_thread([&context, user_ptr, &endpoint, &username, &password, reg_flag]()
                                   { 
                                       user_ptr->connect(endpoint, username, password, reg_flag);
                                       context.run(); 
                                   });

        size_t count = 0;
        while (service_var == 0)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(200ms);
            if (count % 5 == 0)
            {
                printw(".");
                refresh();
            }
            if (count == 25)
            {
                printw("Server is unavailable!");
                getch();
                user.close();
                network_thread.join();
                exit(5);
            }
            ++count;
        }

        if (service_var == -1)
        {
            clear();
            if (reg_flag)
            {
                printw("This user is already exists in the system!");
            }
            else
            {
                printw("You entered uncorrect username or password!");
            }
            getch();
            user.close();
            network_thread.join();
            exit(6);
        }

        ((profile_screen*)manager->scr_profile().get())->init_user_data();

        manager->conn(user_ptr);

        manager->scr_chat_list()->print_title(true);

        manager->start_handle_input();

        user.close();
        network_thread.join();
    }
    catch(std::exception const &err)
    {
        std::cerr << "Error!!! " << err.what() << std::endl;
    }
    endwin();
    return 0;
}