#include "database.hpp"

database::database(std::string &&dbname)
{
    try 
    {
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        
        sql::SQLString url("jdbc:mariadb://127.0.0.1:3306/" + std::move(dbname));
        
        sql::Properties properties(
        {
            {"user", "jancuser"},
            {"password", "xwgb2bm1omsazwdm9hjr"}
        });
        
        std::unique_ptr<sql::Connection> connection{driver->connect(url, properties)};
        conn = std::move(connection);
   }
   catch (sql::SQLException const &e) 
   {
        std::cerr << "Error Connecting to the database: "
                  << e.what() << std::endl;
   }
}

std::optional<idtype> database::get_user_id(strcref username)
{
    try
    {
        sql::SQLString query = "SELECT id FROM user WHERE name = ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setString(1, sql::SQLString(username));

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        if (rs->next())
        {
            idtype user_id = rs->getUInt64(1);

            return std::optional<idtype>{ user_id };
        }
        else
        {
            return std::nullopt;
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with get_user_id function: "
                  << e.what() << std::endl;
        return std::nullopt;

    }
}   

std::optional<idtype> database::authorize(strcref username, strcref password)
{
    try
    {
        sql::SQLString query = "SELECT id, password_hash FROM user WHERE name = ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        if (rs->next())
        {
            idtype      user_id                 = rs->getUInt64(1);
            std::string password_hash_with_salt{  rs->getString(2).c_str() };

            auto [salt, user_password_hash] = split_record(password_hash_with_salt);

            std::string password_hash{ hasher::sha3_512_with_salt(password, salt) };

            return password_hash == user_password_hash ? std::optional<idtype>{ user_id }
                                                       : std::nullopt;
        }
        else
        {
            return std::nullopt;
        }
        
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with authorize function: "
                  << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<idtype> database::add_record(strcref username, strcref password)
{
    try
    {
        sql::SQLString query = "INSERT INTO user(name, password_hash) VALUES (?, ?);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        auto [salt, password_hash] = hasher::sha3_512_with_salt(password);

        stmt->setString(1, sql::SQLString(username));

        stmt->setString(2, sql::SQLString(salt + ":" + password_hash));
        
        stmt->executeUpdate();

        auto new_user_id = last_insert_id("user");

        return new_user_id;
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with add_record function: "
                  << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<message_item> database::save_message(idtype sender_id, idtype chat_id, strcref msg)
{
    try
    {
        sql::SQLString query = "INSERT INTO message(user_id, chat_id, content, creation_time) VALUES (?, ?, ?, ?);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        auto        now = std::chrono::system_clock::now();
        std::time_t t_c = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ts;
	    ts << std::put_time(std::localtime(&t_c), "%F %T");

        stmt->setUInt64(1, sender_id);
        stmt->setUInt64(2, chat_id);
        stmt->setString(3, msg);
        stmt->setString(4, ts.str());

        stmt->executeUpdate();

        auto msg_id = last_insert_id("message");

        if (msg_id.has_value())
        {
            message_item message(*msg_id, msg, chat_id, sender_id, (uint64_t)t_c);
            
            return std::optional<message_item>{message};
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with save_message function: "
                  << e.what() << std::endl;
    }
    return std::nullopt;
}

idtype database::create_chat(strcref chat_name, std::vector<idtype> const &participants_ids, bool is_single_chat)
{
    try
    {
        sql::SQLString query = "INSERT INTO chat(name, is_single_chat) VALUES (?, ?);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setString(1, chat_name);
        stmt->setBoolean(2, is_single_chat);

        stmt->executeUpdate();

        idtype chat_id = *last_insert_id("chat");
        add_participants(chat_id, participants_ids);

        return chat_id;
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with create_chat function: "
                  << e.what() << std::endl;
    }
}

bool database::add_participants(idtype chat_id, std::vector<idtype> const &participants_ids)
{
    try
    {
        sql::SQLString query = "INSERT INTO chat_participant(chat_id, user_id) VALUES (?, ?);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, chat_id);

        for (idtype user_id: participants_ids)
        {
            stmt->setUInt64(2, user_id);

            stmt->executeUpdate();
        }

        return true;
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with add_participants function: "
                  << e.what() << std::endl;
    }
    return false;
}

bool database::delete_chat(idtype chat_id)
{
    try
    {
        sql::SQLString query = "DELETE FROM chat WHERE id = ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, chat_id);

        stmt->executeUpdate();

        return true;
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with delete_chat function: "
                  << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> database::get_usernames(std::vector<idtype> const &ids)
{
    std::vector<std::string> usernames;
    try
    {
        sql::SQLString query = "SELECT name FROM user WHERE id = ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        for (auto const &id: ids)
        {
            stmt->setUInt64(1, id);
            std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

            if (rs->next())
            {
                std::string username = (std::string)rs->getString(1);
                usernames.push_back(username);
            }
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with get_usernames function: "
                  << e.what() << std::endl;
    }
    return usernames;
}

chat_data_t database::get_chats(idtype user_id)
{
    std::vector<idtype> ids;
    std::vector<std::string> names;
    std::vector<idtype> is_single_indicators;
    try
    {
        sql::SQLString query = "SELECT chat.id, chat.name, chat.is_single_chat FROM chat, chat_participant WHERE (chat_participant.user_id = ?) AND (chat.id = chat_participant.chat_id);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, user_id);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        while (rs->next())
        {
            idtype chat_id         = rs->getUInt64(1);
            std::string chatname   = (std::string)rs->getString(2);
            idtype is_single_chat  = (idtype)rs->getBoolean(3);

            ids.push_back(chat_id);
            names.push_back(chatname);
            is_single_indicators.push_back(is_single_chat);
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with get_chat_names function: "
                  << e.what() << std::endl;
    }
    return std::make_tuple(ids, names, is_single_indicators);
}

idname_t database::get_chat_participants(idtype chat_id)
{
    std::vector<idtype>      ids;
    std::vector<std::string> names;
    try
    {
        sql::SQLString query = "SELECT user.id, user.name FROM user, chat_participant WHERE (chat_participant.chat_id = ?) AND (user.id = chat_participant.user_id);";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, chat_id);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        while (rs->next())
        {
            idtype      user_id  = rs->getUInt64(1);     
            std::string username = (std::string)rs->getString(2);
            ids.push_back(user_id);
            names.push_back(username);
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with get_chat_participants function: "
                  << e.what() << std::endl;
    }
    return std::make_pair(ids, names);
}

idname_t database::search_users(strcref search_str, size_t quantity)
{
    std::vector<idtype>      ids;
    std::vector<std::string> names;
    try
    {
        sql::SQLString query = "SELECT id, name FROM user "
                               "WHERE INSTR(name, ?) = 1 OR "
                                     "RIGHT(name, ?) = ? OR "
                                     "INSTR(name, ?) != 0 "
                                     "LIMIT ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setString(1, search_str);
        stmt->setInt(2, search_str.length());
        stmt->setString(3, search_str);
        stmt->setString(4, search_str);
        stmt->setUInt64(5, quantity);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        while (rs->next())
        {
            idtype      user_id  = rs->getUInt64(1);     
            std::string username = (std::string)rs->getString(2);
            ids.push_back(user_id);
            names.push_back(username);
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with search_users function: "
                  << e.what() << std::endl;
    }
    return std::make_pair(ids, names);
}

bool database::change_user_name(idtype user_id, strcref new_username)
{
    try
    {
        sql::SQLString query = "UPDATE user SET name = ? WHERE id = ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setString(1, new_username);
        stmt->setUInt64(2, user_id);

        stmt->executeUpdate();
        
        return true;        
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with change_user_name function: "
                  << e.what() << std::endl;
        return false;
    }
}

bool database::change_password(idtype user_id, strcref new_password)
{
    try
    {
        sql::SQLString query = "UPDATE user SET password_hash = ? WHERE id = ?;";
        
        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        auto [salt, password_hash] = hasher::sha3_512_with_salt(new_password);

        stmt->setString(1, salt + ":" + password_hash);
        stmt->setUInt64(2, user_id);

        stmt->executeUpdate();  
        return true; 
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with change_password function: "
                  << e.what() << std::endl;
    }
    return false;
}

std::vector<message_item> database::get_messages(idtype chat_id, idtype last_msg_id, size_t count)
{
    std::vector<message_item> messages;
    try
    {
        sql::SQLString query = "SELECT * FROM message WHERE (chat_id = ?) AND (id < ?) ORDER BY creation_time LIMIT ?;";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, chat_id);
        stmt->setUInt64(2, last_msg_id);
        stmt->setUInt64(3, (uint64_t)count);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        while (rs->next())
        {
            idtype msg_id  = rs->getUInt64(1);
            std::string content = (std::string)rs->getString(2);
            idtype chat_id = rs->getUInt64(3);
            idtype user_id = rs->getUInt64(4);
            uint64_t creation_time = (uint64_t)str_to_time_t((std::string)rs->getString(5));

            message_item message(msg_id, content, chat_id, user_id, creation_time);
            messages.push_back(message);
        }
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with get_messages function: "
                  << e.what() << std::endl;
    }
    return messages;
}

bool database::is_user_chat_participant(idtype user_id, idtype chat_id)
{
    try
    {
        sql::SQLString query = "SELECT EXISTS(SELECT * FROM chat_participant WHERE (user_id = ?) AND (chat_id = ?));";

        std::unique_ptr<sql::PreparedStatement> stmt{ conn->prepareStatement(query) };

        stmt->setUInt64(1, user_id);
        stmt->setUInt64(2, chat_id);

        std::unique_ptr<sql::ResultSet> rs{ stmt->executeQuery() };

        return rs->next();
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with is_user_chat_participant function: "
                  << e.what() << std::endl;
        return false;
    }
}

std::optional<idtype> database::last_insert_id(strcref table_name)
{
    try
    {    
        sql::SQLString query = "SELECT LAST_INSERT_ID() FROM " + table_name + ";";
        
        std::unique_ptr<sql::Statement> last_id_stmt{ conn->createStatement()};

        std::unique_ptr<sql::ResultSet> rs{ last_id_stmt->executeQuery(query) };

        return rs->next() ? std::optional<idtype>{rs->getUInt64(1)} : std::nullopt;
    }
    catch (sql::SQLException const &e)
    {
        std::cerr << "Error with is_user_chat_participant function: "
                  << e.what() << std::endl;
    }
    return std::nullopt;
}

std::pair<std::string, std::string> database::split_record(strcref record)
{
    for (size_t i = 0; i < record.size(); ++i)
    {
        if (record[i] == ':')
        {
            std::string salt{ record.c_str(), i };
            std::string password_hash{ record.c_str() + i + 1, record.size() - i - 1 };
            return std::make_pair(salt, password_hash);
        }
    }
    std::cout << "!!!ERROR: incorrect record \"" << record << "\"" << std::endl;
}

std::time_t database::str_to_time_t(strcref strtime)
{
    std::tm t = {};
    strptime(strtime.c_str(), "%F %T", &t);
    return std::mktime(&t); 
}