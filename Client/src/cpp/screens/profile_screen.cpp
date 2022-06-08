#include "../../hpp/screens/profile_screen.hpp"

profile_screen::profile_screen(std::weak_ptr<screen_manager> scr_manager, scr_size_data size_data)
: app_screen(scr_manager, size_data)
, win_username{newwin(1, size_data.width-14, size_data.starty+1, 	 size_data.startx+12)}
, win_password_len{newwin(1, size_data.width-14, size_data.starty+3, size_data.startx+12)}
// , panel_username{new_panel(win_username)}
// , panel_password_len{new_panel(win_password_len)}
, line_username{win_username, 32}
, line_password{win_password_len, 4000, true}
, curline{0}
{}

void profile_screen::init_user_data()
{
    keypad(win_username, true);
    keypad(win_password_len, true);
    if (auto manager_p = manager.lock())
    {
        title = "Profile info";
        auto [user_name, password_length] = manager_p->db()->get_user_data();
        
        username = user_name;
        password_len = password_length;
        line_username.line(user_name);
        line_password.line(std::string(password_length, '*'));
    }
}

void profile_screen::handle_input(int c)
{
    if (!is_changing)
    {
        switch (c)
        {
            case KEY_BTAB:
            case KEY_CTAB:
            case KEY_STAB:
            case KEY_CATAB:
            case 9:
            {
                if (curline == 0) { curline = 1; }
                else { curline = 0; }
            }
            break;
            case KEY_ENTER:
            case 10:
            {
                is_changing = true;
                if (curline == 0) 
                { 
                    line_username.print_line();
                }
                else
                {
                    line_password.clear();
                    line_password.print_line();
                }
            }
            break;
            case KEY_UP:
            {
                if (curline == 1) { curline = 0; }
            }
            break;
            case KEY_DOWN:
            {
                if (curline == 0) { curline = 1; }
            }
            break;
        }
        show();
    }
    else
    {
        if (c == 10 || c == KEY_ENTER)
        {
            if (curline == 0)
            {
                std::string new_username = line_username.line();
                
                if (auto manager_p = manager.lock(); new_username != username)
                {
                    if (!is_allowed_chars(new_username))
                    {
                        manager_p->scr_dialog()->show();
                        
                        return;
                    }
                    manager_p->db()->update_user_name(new_username);
                    
                    message_parser::parsed_message message_data;
                    message_data.type    = message_parser::message_types::CHANGE_USER_NAME;
                    message_data.strargs = {new_username};
                    message_data.idargs.clear();
                    
                    chat_message sending_msg = message_parser::parsed_to_chat_message(message_data);
                    manager_p->conn()->send_msg(sending_msg);
                    
                    change_username(new_username);
                }
            }
            else
            {
                size_t new_password_len = line_password.line_len();
                
                if (new_password_len == 0)
                {
                    line_password.line(std::string(password_len, '*'));
                    is_changing = false;
                    return;
                }

                std::string new_password = line_password.line();

                if (auto manager_p = manager.lock())
                {
                    manager_p->db()->update_user_password_len(new_password_len);

                    message_parser::parsed_message message_data;
                    message_data.type    = message_parser::message_types::CHANGE_PASSWORD;
                    message_data.strargs = {new_password};
                    message_data.idargs.clear();

                    chat_message sending_msg = message_parser::parsed_to_chat_message(message_data);
                    manager_p->conn()->send_msg(sending_msg);
                    
                    change_password_len(new_password_len);
                }
            }
            is_changing = false;
        }
        else
        {
            if (curline == 0)
            {
                line_username.handle_input(c);
                line_username.print_line();
            }
            else
            {
                line_password.handle_input(c);
                line_password.print_line();
            }
        }
    }
}

void profile_screen::show()
{
    if (auto manager_p = manager.lock())
    {
        size_t height, width;
        getmaxyx(stdscr, height, width);
        scr_size_data size_data{height/2, width/2, height/4, width/4};

        manager_p->set_scr(std::make_shared<dialog_screen>(false, manager, size_data, 
                std::bind(&profile_screen::dialog_responce_handler, this, std::placeholders::_1)),
        screen_manager::scrnums::DIALOG);
        ((dialog_screen*)manager_p->scr_dialog().get())->message("Only the characters a-z, A-Z, 0-9 and _ are available!");
        manager_p->scr_dialog()->hide();
    }

    if (curline == 0)
    {
        mvwprintw(winparts[1], 0, 0, "*");
        mvwprintw(winparts[1], 2, 0, " ");
    }
    else
    {
        mvwprintw(winparts[1], 2, 0, "*");
        mvwprintw(winparts[1], 0, 0, " ");
    }
    
    mvwprintw(winparts[1], 0, 1, "USERNAME:|");
    mvwprintw(winparts[1], 2, 1, "PASSWORD:|");
    mvwprintw(winparts[1], 0, width-1, "|");
    mvwprintw(winparts[1], 2, width-1, "|");
    
    wclrtoeol(winparts[1]);

    app_screen::show();

    if (curline == 0)
    {
        line_password.print_line();
        line_username.print_line();
    }
    else
    {
        line_username.print_line();
        line_password.print_line();
    }

    if (!is_changing)
    {
        wmove(winparts[1], curline*2, 0);
        wrefresh(winparts[1]);
    }
}

void profile_screen::hide()
{
    line_username.line(username);
    line_password.line(std::string(password_len, '*'));
    is_changing = false;
    curline = 0;
}

void profile_screen::change_username(std::string const &new_username)
{
    username = new_username;
    line_username.line(username);
}

void profile_screen::change_password_len(size_t new_len)
{
    password_len = new_len;
    line_password.line(std::string(password_len, '*'));
}

bool profile_screen::is_allowed_chars(std::string const &str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { 
    unsigned char char_num = static_cast<unsigned char>(c);
    return (char_num == 95) || ((char_num > 47) && (char_num < 58)) || 
          ((char_num > 64) && (char_num < 91)) || ((char_num > 96) && (char_num < 123));
    });
}

void profile_screen::dialog_responce_handler(int)
{
    if (curline == 0)
    {
        line_password.print_line();
        line_username.print_line();
    }
    else
    {
        line_username.print_line();
        line_password.print_line();
    }
}