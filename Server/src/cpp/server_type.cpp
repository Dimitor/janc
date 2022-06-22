#include "../hpp/server_type.hpp"

int server_type::join(uint64_t connection_id, connection_ptr connection)
{
    auto search_it = connections.find(connection_id);
    
    if (search_it != connections.end())
    {
        if (search_it->second.size() == 5)
        {
            return 1;
        }
        search_it->second.push_back(connection);
    }
    else
    {
        connections.emplace(connection_id, std::vector{ connection });
    }
    return 0;
}

int server_type::leave(uint64_t connection_id, connection_ptr connection)
{
    auto search_it = connections.find(connection_id);
    
    auto &user_connections = search_it->second;
    
    if (user_connections.size() > 1)
    {
        for (auto it = user_connections.begin(); it < user_connections.end(); ++it)
        {
            if (*it == connection)
            {
                user_connections.erase(it);
                break;
            }
        }
    }
    else
    {
        connections.erase(search_it);
    }
    return 0;
}

bool server_type::process_message(chat_message const &msg, connection_ptr connection)
{
    using message_types = message_parser::message_types;

    auto message_info = message_parser::parse(msg);
    auto idargs       = message_info.idargs;
    auto strargs      = message_info.strargs; 

    switch (message_info.type)
    {
        case message_types::MESSAGE: 
        {
            auto chat_id = idargs[0];
            auto content = strargs[0];

            auto message = *(db.save_message(connection->id(), chat_id, content));

            std::stringstream ts;
            std::time_t t_c = (std::time_t)message.creation_time;
	        ts << std::put_time(std::localtime(&t_c), "%F %T");

            message_info.idargs = {message.id, message.user_id, message.chat_id, message.creation_time};
            message_info.strargs = {message.content};

            auto [participant_ids, _participant_names] = db.get_chat_participants(chat_id);

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, participant_ids);
        }
        break;
        case message_types::LOAD_MESSAGES:
        {
            auto chat_id     = idargs[0];
            auto last_msg_id = idargs[1];
            auto count       = idargs[2];

            auto messages = db.get_messages(chat_id, last_msg_id, count);

            for (auto msg_it = messages.begin(); msg_it != messages.end(); ++msg_it)
            {
                message_info.idargs = {(*msg_it).id, (*msg_it).user_id, (*msg_it).chat_id, (*msg_it).creation_time}; 
                message_info.strargs = {(*msg_it).content};

                chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

                send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
            }
        }
        break;
        case message_types::LOAD_CHATS:
        {
            auto [chat_ids, chat_names, chat_single_indicators] = db.get_chats(connection->id());

            if (!chat_ids.empty())
            {
                message_info.idargs = {0, 0};
                message_info.strargs = {""};
                for (size_t i = 0; i < chat_ids.size(); ++i)
                {
                    message_info.idargs[0] = chat_ids[i];
                    message_info.idargs[1] = chat_single_indicators[i];
                    message_info.strargs[0] = chat_names[i];
                    
                    chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

                    send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
                }
            }
            
            message_info.type = message_types::END_LOADING;
            message_info.idargs.clear();
            message_info.strargs.clear();

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
        }
        break;
        case message_types::LOAD_PARTICIPANTS:
        {
            auto chat_id = idargs[0];
            
            auto [participant_ids, participant_names] = db.get_chat_participants(chat_id);

            message_info.idargs  = {participant_ids.size(), chat_id};

            for (auto id: participant_ids)
            {
               message_info.idargs.push_back(id); 
            }

            message_info.strargs = participant_names;

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
        }
        break;
        case message_types::SEARCH_USERS:
        {
            auto quantity = idargs[0];
            auto search_str = strargs[0];

            auto [user_ids, user_names] = db.search_users(search_str, quantity);

            message_info.idargs = {user_ids.size()};
            for (auto id: user_ids)
            {
               message_info.idargs.push_back(id); 
            }
            message_info.strargs = user_names;

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
        }
        break;
        case message_types::CREATE_SINGLE_CHAT:
        {
            auto second_participant = idargs[0];

            std::vector<uint64_t> participant_ids = {connection->id(), second_participant};

            auto participant_names = db.get_usernames(participant_ids);

            auto chat_name = participant_names[0] + ":" + participant_names[1];

            auto chat_id = db.create_chat(chat_name, participant_ids, true);
            
            message_info.idargs  = {chat_id, participant_ids[0], participant_ids[1]};
            message_info.strargs = {participant_names[0], participant_names[1]};

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, participant_ids);
        }
        break;
        case message_types::CREATE_GROUP:
        {
            auto chat_name       = strargs[0];
            auto participant_ids = idargs;
            participant_ids.push_back(connection->id());

            auto chat_id = db.create_chat(chat_name, participant_ids, false);

            auto participant_names = db.get_usernames(participant_ids);

            message_info.idargs = {participant_ids.size()+1, chat_id};

            for (auto id: participant_ids)
            {
                message_info.idargs.push_back(id);    
            }

            message_info.strargs.clear();

            for (auto &name: participant_names)
            {
                message_info.strargs.push_back(name);
            }

            message_info.strargs.push_back(chat_name);

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            send_msg_to_chat_participants(sending_msg, participant_ids);
        }
        break;
        case message_types::DELETE_CHAT:
        {
            auto chat_id = idargs[0];

            bool is_participant = db.is_user_chat_participant(connection->id(), chat_id);

            if (!is_participant) { return false; }

            auto [participant_ids, _participant_names] = db.get_chat_participants(chat_id);

            send_msg_to_chat_participants(msg, participant_ids);

            db.delete_chat(chat_id);
        }
        break;
        case message_types::CHANGE_USER_NAME:
        {
            auto new_username = strargs[0];

            db.change_user_name(connection->id(), new_username);
            
            message_info.idargs.clear();

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);
            
            send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
        }
        break;
        case message_types::CHANGE_PASSWORD:
        {
            auto new_password = strargs[0];

            db.change_password(connection->id(), new_password);
            
            auto password_len = new_password.length();

            message_info.idargs = {password_len};

            message_info.strargs.clear();
            
            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);
            
            send_msg_to_chat_participants(sending_msg, std::vector{connection->id()});
        }
        break;
        default: return false;
    }
    return true;
}

bool server_type::process_record(chat_message const &rec_msg, connection_ptr connection)
{
    using message_types = message_parser::message_types;

    auto message_info = message_parser::parse(rec_msg);
    auto strargs      = message_info.strargs; 

    switch (message_info.type)
    {
        case message_types::AUTHORIZE_USER:
        {
            auto username = strargs[0];
            auto password = strargs[1];
            
            auto user_id = db.authorize(username, password);
            auto join_res = 1;
            if (user_id.has_value())
            {
                connection->id(*user_id);
                if (join(*user_id, connection) == 1)
                {
                    join_res = 2;
                }
                else 
                {
                    join_res = 0;
                }
            }

            message_info.type = message_types::AUTHORIZE_USER;
            message_info.idargs.clear();
            message_info.strargs.clear();
            message_info.idargs.push_back(join_res);

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            connection->send_msg(sending_msg);

            return join_res == 0;
        }
        case message_types::REGISTER_USER:
        {
            auto username = strargs[0];
            auto password = strargs[1];
            
            auto user_id = db.get_user_id(username);
            bool user_exists = user_id.has_value();
            if (!user_exists) 
            {
                auto new_user_id = db.add_record(username, password);
                connection->id(*new_user_id);
                join(*new_user_id, connection);
            }
            message_info.type = message_types::REGISTER_USER;
            message_info.idargs.clear();
            message_info.strargs.clear();
            message_info.idargs.push_back((uint64_t)user_exists);

            chat_message sending_msg = message_parser::parsed_to_chat_message(message_info);

            connection->send_msg(sending_msg);

            return !user_exists;
        }
        default: return false;
    }
}

void server_type::send_msg_to_chat_participants(chat_message const &msg, std::vector<uint64_t> const &chat_participants)
{

    for (auto participant_id: chat_participants)
    {
        auto search_it = connections.find(participant_id);
        if (search_it != connections.end())
        {
            for (auto conn: search_it->second)
            {
                conn->send_msg(msg);
            }
        }
    }
}