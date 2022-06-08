#pragma once
#include <memory>
#include <iostream>
#include "panel.h"
#include "../scr_size_data.hpp"

struct screen_manager;

struct app_screen
{
    app_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data);
    virtual ~app_screen(){}

    virtual void handle_input(int c){}

    bool is_show() { return showed; }

    virtual void print_title(std::string const &new_title, bool is_selected=false);
    virtual void print_title(bool is_selected=false);

    virtual void show();

    virtual void hide();

protected:

    WINDOW *winparts[2];
    PANEL *panels[2];
    std::weak_ptr<screen_manager> manager;
    std::string title;
    bool showed;
    size_t height, width;
};