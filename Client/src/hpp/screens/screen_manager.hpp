#pragma once
#include <memory>
#include "panel.h"
#include "app_screen.hpp"
#include "../database.hpp"
#include "../connection_type.hpp"
#include "../scr_size_data.hpp"
#define ctrl(x) ((x) & 0x1f)

struct screen_manager: std::enable_shared_from_this<screen_manager>
{
    screen_manager(std::shared_ptr<database> app_db);
    ~screen_manager(){ endwin(); }

    enum scrnums
    {
        CHAT_LIST     = 0,
        MESSAGE_INPUT = 1,
        MESSAGE_LIST  = 2,
        CHAT_INFO     = 3,
        PROFILE_INFO  = 4,
        CREATE_CHAT   = 5,
        DIALOG        = 6
    };

    std::shared_ptr<app_screen> scr_chat_list()     { return screens[scrnums::CHAT_LIST]; }
    std::shared_ptr<app_screen> scr_message_input() { return screens[scrnums::MESSAGE_INPUT]; }
    std::shared_ptr<app_screen> scr_message_list()  { return screens[scrnums::MESSAGE_LIST]; }
    std::shared_ptr<app_screen> scr_chat_info()     { return screens[scrnums::CHAT_INFO]; }
    std::shared_ptr<app_screen> scr_profile()       { return screens[scrnums::PROFILE_INFO]; }
    std::shared_ptr<app_screen> scr_create_chat()   { return screens[scrnums::CREATE_CHAT]; }
    std::shared_ptr<app_screen> scr_dialog()        { return screens[scrnums::DIALOG]; }

    void set_scr(std::shared_ptr<app_screen> &&scr, int scr_num) {screens[scr_num] = std::move(scr);}

    std::vector<std::shared_ptr<app_screen>> get_screens(){return screens;}

    void start_handle_input();
    
    int scr_num() const { return curscr_num; }
    void scr_num(int new_num);

    void conn(std::shared_ptr<connection_type> new_conn) { connection = new_conn; }

    std::shared_ptr<connection_type> conn() const { return connection; }

    std::shared_ptr<database> db() const { return manager_db; }

private:


    std::vector<std::shared_ptr<app_screen>> screens;

    std::shared_ptr<database> manager_db;

    std::shared_ptr<connection_type> connection;

    int curscr_num;
    int prevscr_num;
};