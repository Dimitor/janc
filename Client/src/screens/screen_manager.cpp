#include "screen_manager.hpp"

screen_manager::screen_manager(std::shared_ptr<database> app_db)
: manager_db{app_db}
, screens(7)
, curscr_num{0}
{}

void screen_manager::scr_num(int new_num)
{ 
    curscr_num = new_num; 
}

void screen_manager::start_handle_input()
{
    while(1)
    {
        int input = wgetch(stdscr);

        if (input == ctrl('c'))
        {
            clear();
            refresh();
            exit(0);
        }

        switch (input)
        {
            case KEY_BTAB:
            case KEY_CTAB:
            case KEY_STAB:
            case KEY_CATAB:
            case 9:
            {
                switch (curscr_num)
                {
                    case scrnums::MESSAGE_INPUT:
                    {
                        scr_message_input()->print_title(false);
                        scr_message_list()->print_title(true);
                        curscr_num = scrnums::MESSAGE_LIST;
                    }
                    break;
                    case scrnums::MESSAGE_LIST:
                    {
                        scr_message_list()->print_title(false);
                        if (scr_chat_info()->is_show())
                        {
                            scr_chat_info()->print_title(true);
                            curscr_num = scrnums::CHAT_INFO;
                        }
                        else
                        {
                            scr_message_input()->print_title(true);
                            curscr_num = scrnums::MESSAGE_INPUT;
                        }
                    }
                    break;
                    case scrnums::CHAT_INFO:
                    {
                        scr_chat_info()->print_title(false);
                        scr_message_input()->print_title(true);
                        curscr_num = scrnums::MESSAGE_INPUT;
                    }
                    break;
                    default: screens[curscr_num]->handle_input(input);
                }
            }
            break;
            case ctrl('a'):
            {
                if (curscr_num == scrnums::MESSAGE_INPUT || curscr_num == scrnums::MESSAGE_LIST)
                {
                    if (!scr_chat_info()->is_show())
                    {
                        scr_chat_info()->print_title(false);
                        scr_chat_info()->show();
                    }
                    else
                    {
                        scr_chat_info()->hide();
                    }
                } 
                else if (curscr_num == scrnums::CHAT_INFO)
                {
                    scr_message_input()->print_title(true);
                    curscr_num = scrnums::MESSAGE_INPUT;
                    scr_chat_info()->hide();
                }
            }
            break;
            case ctrl('p'):
            {
                if (curscr_num == scrnums::PROFILE_INFO)
                {
                    scr_profile()->hide();
                    curscr_num = prevscr_num;
                    screens[curscr_num]->print_title(true);
                }
                else
                {
                    prevscr_num = curscr_num;
                    screens[curscr_num]->print_title(false);
                    curscr_num = scrnums::PROFILE_INFO;
                    scr_profile()->print_title(true);
                }
            }
            break;
            case 'h':
            {
                if (curscr_num == scrnums::CHAT_LIST)
                {
                    scr_dialog()->show();
                }
                else
                {
                    screens[curscr_num]->handle_input(input);
                }
            }
            break;
            case ctrl('n'):
            {
                if (curscr_num == scrnums::CHAT_LIST)
                {
                    scr_create_chat()->print_title(false);
                    curscr_num = scrnums::CREATE_CHAT;
                }
            }
            break;
            case ctrl('l'):
            {
                for (size_t i = 0; i < screens.size(); ++i)
                {
                    if (screens[i])
                    {
                        screens[i]->hide();
                    }
                }
                scr_chat_list()->print_title();
                curscr_num = scrnums::CHAT_LIST;
            }
            break;
            default: 
            {
                screens[curscr_num]->handle_input(input);
            }
        }    
    }   
}