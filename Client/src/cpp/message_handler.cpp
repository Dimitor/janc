#include "../hpp/message_handler.hpp"

bool message_handler::process_message(chat_message const &msg)
{
    using message_types = message_parser::message_types;

    auto message_info  = message_parser::parse(msg);
    auto const idargs  = message_info.idargs;
    auto const strargs = message_info.strargs; 
    switch (message_info.type)
    {
        case message_types::MESSAGE: 
        {
            auto msg_id          = idargs[0];
            auto user_id         = idargs[1];
            auto chat_id         = idargs[2];
            time_t creation_time = (time_t)idargs[3];
            auto message         = strargs[0];

            db->save_message(msg_id, message, chat_id, user_id, creation_time);
            auto msg_prefix = db->get_message_prefix(msg_id);
            
            if (manager->scr_message_list())
            {
                ((message_list_screen*)(manager->scr_message_list().get()))->push_back_msg(chat_id, msg_id, msg_prefix + message);
            }
        }
        break;
        case message_types::LOAD_MESSAGES:
        {
            auto msg_id          = idargs[0];
            auto user_id         = idargs[1];
            auto chat_id         = idargs[2];
            std::time_t creation_time = (std::time_t)idargs[3];
            auto message         = strargs[0];

            db->save_message(msg_id, message, chat_id, user_id, creation_time);
            auto msg_prefix = db->get_message_prefix(msg_id);
            
            if (manager->scr_message_list())
            {
                ((message_list_screen*)(manager->scr_message_list().get()))->push_front_msg(chat_id, msg_id, msg_prefix + message);
            }
            
        }
        break;
        case message_types::LOAD_CHATS:
        {

            idtype chat_id = idargs[0];
            idtype single_indicator = idargs[1];
            std::string chat_name = strargs[0];
            
            if (single_indicator != 0)
            {
                auto delimiter = chat_name.find(":");
                auto [username, password_len] = db->get_user_data();
                if (username == chat_name.substr(delimiter+1))
                {
                    chat_name = chat_name.substr(0, delimiter);
                }
                else
                {
                    chat_name = chat_name.substr(delimiter + 1);
                }
            }

            db->add_chat(chat_id, chat_name);
            ((chat_list_screen*)(manager->scr_chat_list().get()))->add_chat(chat_id, std::move(chat_name));
            chat_message sending_msg;
            sending_msg.from_str(message_types::LOAD_PARTICIPANTS, reinterpret_cast<char*>(&chat_id), sizeof(chat_id));
            if (auto conn_ptr = conn.lock())
            {
                conn_ptr->send_msg(sending_msg);

                message_info.type = message_types::LOAD_MESSAGES;
                message_info.idargs = {chat_id, 1000000000000000, 40};
                message_info.strargs.clear();

                sending_msg = message_parser::parsed_to_chat_message(message_info);

                conn_ptr->send_msg(sending_msg);
            }
        }
        break;
        case message_types::LOAD_PARTICIPANTS:
        {
            auto chat_id = idargs[0];
            for (int i = 1; i < idargs.size(); ++i)
            {
                auto participant_id   = idargs[i];
                auto participant_name = strargs[i - 1];
                db->add_participant(participant_id, participant_name);
                db->add_chat_participant(chat_id, participant_id);
            }
            if ((manager->scr_chat_info()) && (manager->scr_num() == screen_manager::scrnums::CHAT_INFO))
            {
                ((info_screen*)(manager->scr_chat_info().get()))->update_participants();
            }
        }
        break;
        case message_types::SEARCH_USERS:
        {
            using user_item = chat_item;
            auto user_ids   = idargs;
            auto user_names = strargs;

            std::vector<user_item> users;
            for (size_t i = 0; i < user_ids.size(); ++i)
            {
                users.emplace_back(user_ids[i], user_names[i]);
            }
            
            if ((manager->scr_create_chat()) && (manager->scr_num() == screen_manager::scrnums::CREATE_CHAT))
            {
                ((create_chat_screen*)manager->scr_create_chat().get())->update_user_list(std::move(users));
            }
        }
        break;
        case message_types::CREATE_SINGLE_CHAT:
        {
            auto chat_id = idargs[0];
            auto participant1_id = idargs[1];
            auto participant2_id = idargs[2];
            auto participant1_name = strargs[0];
            auto participant2_name = strargs[1];
            
            std::string chat_name;

            auto [username, password_len] = db->get_user_data();
            if (username == participant1_name)
            {
                chat_name = participant2_name;
            }
            else
            {
                chat_name = participant1_name;
            }
            db->add_chat(chat_id, chat_name);

            db->add_participant(participant1_id, participant1_name);
            db->add_chat_participant(chat_id, participant1_id);
            db->add_participant(participant2_id, participant2_name);
            db->add_chat_participant(chat_id, participant2_id);
            
            if (manager->scr_chat_list())
            {
                ((chat_list_screen*)(manager->scr_chat_list().get()))->add_chat(chat_id, std::move(chat_name));
                if (manager->scr_num() == screen_manager::scrnums::CHAT_LIST)
                {
                    manager->scr_chat_list()->print_title(true);
                }
            }
        }
        break;
        case message_types::CREATE_GROUP:
        {
            auto chat_id   = idargs[0];
            auto chat_name = strargs.back();

            db->add_chat(chat_id, chat_name);

            for (size_t i = 1; i < idargs.size(); ++i)
            {
                auto participant_id   = idargs[i];
                auto participant_name = strargs[i-1];
                
                db->add_participant(participant_id, participant_name);
                db->add_chat_participant(chat_id, participant_id);
            }
            
            if (manager->scr_chat_list())
            {
                ((chat_list_screen*)(manager->scr_chat_list().get()))->add_chat(chat_id, std::move(chat_name));
                if (manager->scr_num() == screen_manager::scrnums::CHAT_LIST)
                {
                    manager->scr_chat_list()->print_title(true);
                }
            }
        }
        break;
        case message_types::DELETE_CHAT:
        {
            auto chat_id = idargs[0];
            db->delete_chat(chat_id);
            if (manager->scr_chat_list())
            {
                ((chat_list_screen*)(manager->scr_chat_list().get()))->delete_chat(chat_id);
            }
        }
        break;
        case message_types::CHANGE_USER_NAME:
        {
            auto new_username = strargs[0];
            
            db->update_user_name(new_username);
            if (manager->scr_profile())
            {
                ((profile_screen*)(manager->scr_profile().get()))->change_username(new_username);
            }
        }
        break;
        case message_types::CHANGE_PASSWORD:
        {
            auto new_password_len = idargs[0];

            db->update_user_password_len(new_password_len);
            if (manager->scr_profile())
            {
                ((profile_screen*)(manager->scr_profile().get()))->change_password_len(new_password_len);
            }
        }
        break;
        case message_types::END_LOADING:
        {
            service_var = 1;
        }
        break;
        case message_types::AUTHORIZE_USER:
        case message_types::REGISTER_USER:
        {
            int authentication_res = static_cast<int>(idargs[0]);

            if (authentication_res == 0)
            {
                db->insert_user_data(username, password_len);    
                message_info.type = message_types::LOAD_CHATS;
                message_info.idargs.clear();
                message_info.strargs.clear();
                chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);
                if (auto conn_ptr = conn.lock())
                {
                    conn_ptr->send_msg(sending_msg);
                }
            }
            else
            {
                service_var = -authentication_res;
            }
        }
        break;
        default: return false;
    }
    return true;
}