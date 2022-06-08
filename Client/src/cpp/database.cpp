#include "../hpp/database.hpp"

#include <string>

database::database(std::string &&dbname)
{
    auto err = sqlite3_open((dbname + ".db").c_str(), &conn);
    
    if (err) 
    {
        std::cout << "DB Error: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_close(conn);
    }

    std::string query = "CREATE TABLE IF NOT EXISTS userdata ("
        "name         VARCHAR(32) PRIMARY KEY NOT NULL UNIQUE,"
        "password_len INT8                     NOT NULL);";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    query = "CREATE TABLE IF NOT EXISTS chat ("
        "id   INT8 PRIMARY KEY NOT NULL,"       
        "name VARCHAR(255)     NOT NULL UNIQUE);";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    query = "CREATE TABLE IF NOT EXISTS participant ("
        "id   INT8 PRIMARY KEY NOT NULL,"       
        "name VARCHAR(32)     NOT NULL UNIQUE);";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    query = "CREATE TABLE IF NOT EXISTS chat_participant ("
        "chat_id        INT8 NOT NULL,"
        "participant_id INT8 NOT NULL,"
        "PRIMARY KEY    (participant_id, chat_id),"
        "FOREIGN KEY    (participant_id) REFERENCES participant(id),"
        "FOREIGN KEY    (chat_id) REFERENCES chat(id));";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    query = "CREATE TABLE IF NOT EXISTS message ("
            "id             INT8 PRIMARY KEY NOT NULL, "
            "content        TEXT NOT NULL, "
            "chat_id        INT8  NOT NULL, "
            "participant_id INT8  NOT NULL, "
            "creation_time  INT8  NOT NULL, "
            "FOREIGN KEY    (participant_id) REFERENCES participant(id), "
            "FOREIGN KEY    (chat_id) REFERENCES chat(id));";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    query = "CREATE TRIGGER del_chat_data BEFORE DELETE "
            "ON chat "
            "BEGIN "
            "DELETE FROM message WHERE message.chat_id = OLD.id; "
            "DELETE FROM chat_participant WHERE chat_participant.participant_id = OLD.id; "
            "END;";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);
}

std::vector<message_item> database::get_messages(idtype chat_id, size_t lborder, size_t rborder)
{
    std::string query = "SELECT message.id, message.content, participant.name, message.creation_time "
                        "FROM message, participant "
                        "WHERE (message.chat_id = " + std::to_string(chat_id) + ") AND (message.participant_id = participant.id) "
                        "ORDER BY message.creation_time "
                        "LIMIT " + std::to_string(lborder) + "," + std::to_string(rborder) + ";";
    messages.clear();
    sqlite3_exec(conn, query.c_str(), get_messages_callback_static, this, &err_msg);
    return messages;
}

std::string database::get_message_prefix(idtype msg_id)
{
    std::string query = "SELECT datetime(message.creation_time,'unixepoch'), participant.name "
                        "FROM message, participant "
                        "WHERE (message.id = " + std::to_string(msg_id) + ") AND (participant.id = message.participant_id);";
    msg_prefix.clear();
    sqlite3_exec(conn, query.c_str(), get_message_prefix_callback_static, this, &err_msg);
    return msg_prefix;
}

int database::get_message_quantity(idtype chat_id)
{
    std::string query = "SELECT COUNT(*) FROM message WHERE chat_id = " + std::to_string(chat_id) + ";";
    sqlite3_exec(conn, query.c_str(), int_res_callback_static, this, &err_msg);
    return int_res;
}

std::vector<std::string> database::get_chat_participants(idtype chat_id)
{
    std::string query = "SELECT participant.name "
                        "FROM participant, chat_participant "
                        "WHERE (chat_id = " + std::to_string(chat_id) + ") AND "
                        "(chat_participant.participant_id = participant.id);";
    strres_vec.clear();
    sqlite3_exec(conn, query.c_str(), get_strres_vec_callback_static, this, &err_msg);
    return strres_vec;
}

bool database::chat_exists(idtype chat_id)
{
    std::string query = "SELECT EXISTS(SELECT * FROM chat where id = " + std::to_string(chat_id) + ");";
    
    sqlite3_exec(conn, query.c_str(), boolres_callback_static, this, &err_msg);
    return boolres;
}

std::vector<chat_item> database::get_chats()
{
    std::string query = "SELECT * FROM chat;";

    sqlite3_exec(conn, query.c_str(), get_chats_callback_static, this, &err_msg);
    return chats;
}

int database::save_message(idtype msg_id, strcref content, idtype chat_id, idtype participant_id, std::time_t creation_time)
{
    std::string query = "INSERT INTO message('id', 'content', 'chat_id', 'participant_id', 'creation_time') "
                        "VALUES (?, ?, ?, ?, strftime('%s', ?));";

    std::stringstream ts;
	ts << std::put_time(std::localtime(&creation_time), "%F %T");

    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    std::string time_str = ts.str();

    sqlite3_bind_int64(stmt, 1, msg_id);
    sqlite3_bind_text(stmt, 2, content.c_str(), content.length(), SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, chat_id);
    sqlite3_bind_int64(stmt, 4, participant_id);
    sqlite3_bind_text(stmt, 5, time_str.c_str(), time_str.length(), SQLITE_STATIC);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);

    return 0;
}

int database::clear_messages(idtype chat_id)
{
    std::string query = "DELETE FROM message WHERE chat_id = " + std::to_string(chat_id) + ";";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);
    return 0;
}

int database::add_chat(idtype chat_id, strcref chat_name)
{
    std::string query = "INSERT INTO chat('id', 'name') VALUES (?, ?);";

    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, chat_name.c_str(), chat_name.length(), SQLITE_STATIC);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);

    return 0;
}

int database::add_chat_participant(idtype chat_id, idtype participant_id)
{
    std::string query = "INSERT INTO chat_participant('chat_id', 'participant_id') VALUES (?, ?);";

    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, chat_id);
    sqlite3_bind_int64(stmt, 2, participant_id);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);
    return 0;
}

int database::add_participant(idtype participant_id, strcref participant_name)
{
    std::string query = "INSERT INTO participant('id', 'name') VALUES (?, ?);";

    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, participant_id);
    sqlite3_bind_text(stmt, 2, participant_name.c_str(), participant_name.length(), SQLITE_STATIC);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);
    return 0;
}

int database::delete_chat(idtype chat_id)
{
    std::string query = "DELETE FROM chat WHERE chat.id = " + std::to_string(chat_id) + ";";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);

    return 0;
}

std::tuple<std::string, size_t> database::get_user_data()
{
    std::string query = "SELECT name, password_len FROM userdata;";
    sqlite3_exec(conn, query.c_str(), get_user_data_callback_static, this, &err_msg);

    return std::make_tuple(str_res, int_res);
}

bool database::user_exists()
{
    std::string query = "SELECT EXISTS(SELECT * FROM userdata);";
    sqlite3_exec(conn, query.c_str(), boolres_callback_static, this, &err_msg);

    return boolres;
}

std::string database::get_chat_name(idtype chat_id)
{
    std::string query = "SELECT name FROM chat WHERE id = " + std::to_string(chat_id) + ";";
    sqlite3_exec(conn, query.c_str(), get_strres_callback_static, this, &err_msg);

    return str_res;
}

int database::clear_db(bool with_user)
{
    std::string del_query;
    if (with_user)
    {
        del_query = "DELETE FROM userdata;";    
        sqlite3_exec(conn, del_query.c_str(), default_callback_static, this, &err_msg);
    }

    del_query = "DELETE FROM message;";
    sqlite3_exec(conn, del_query.c_str(), default_callback_static, this, &err_msg);

    del_query = "DELETE FROM chat_participant;";
    sqlite3_exec(conn, del_query.c_str(), default_callback_static, this, &err_msg);

    del_query = "DELETE FROM chat;";
    sqlite3_exec(conn, del_query.c_str(), default_callback_static, this, &err_msg);

    del_query = "DELETE FROM participant;";
    sqlite3_exec(conn, del_query.c_str(), default_callback_static, this, &err_msg);

    return 0;
}

int database::insert_user_data(strcref username, int password_len)
{
    std::string query = "INSERT INTO userdata('name','password_len') VALUES(?, ?);";
    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, password_len);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);
    return 0;
}

int database::update_user_name(strcref new_name)
{
    std::string query = "UPDATE userdata SET name = ?;";
    sqlite3_prepare_v2(conn, query.c_str(), query.length(), &stmt, NULL);

    sqlite3_bind_text(stmt, 1, new_name.c_str(), new_name.length(), SQLITE_STATIC);

    auto err = sqlite3_step(stmt);

	sqlite3_finalize(stmt);
    return 0;
}

int database::update_user_password_len(int password_len)
{
    std::string query = "UPDATE userdata SET 'password_len' = " + std::to_string(password_len) + ";";

    sqlite3_exec(conn, query.c_str(), default_callback_static, this, &err_msg);
    return 0;
}

int database::int_res_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->int_res_callback(argc, argv, azColName);

    return 0;
}

int database::get_messages_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_messages_callback(argc, argv, azColName);

    return 0;
}

int database::get_message_prefix_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_message_prefix_callback(argc, argv, azColName);

    return 0;
}

int database::boolres_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->boolres_callback(argc, argv, azColName);

    return 0;
}

int database::get_user_data_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_user_data_callback(argc, argv, azColName);

    return 0;
}

int database::get_strres_vec_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_strres_vec_callback(argc, argv, azColName);

    return 0;
}

int database::get_strres_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_strres_callback(argc, argv, azColName);

    return 0;
}

int database::get_chats_callback_static(void *instance, int argc, char **argv, char **azColName)
{
    database *dbconn = static_cast<database *>(instance);
    dbconn->get_chats_callback(argc, argv, azColName);

    return 0;
}

int database::int_res_callback(int argc, char **argv, char **azColName)
{
    int_res = std::stoi(argv[0]);
    return 0;
}

int database::get_message_prefix_callback(int argc, char **argv, char **azColName)
{
    std::string creation_time{   argv[0]};
    std::string participant_name{argv[1]};

    msg_prefix = "[" + creation_time + "|" + participant_name + "]";
    
    return 0;
}

int database::get_messages_callback(int argc, char **argv, char **azColName)
{
    idtype msg_id           = std::stoll(argv[0]);  
    std::string content     = {argv[1]};
    std::string sender_name = {argv[2]};
    uint64_t creation_time  = std::stoll(argv[3]);

    messages.emplace_back(msg_id, content, sender_name, creation_time);
    
    return 0;
}

int database::boolres_callback(int argc, char **argv, char **azColName)
{
    boolres = (bool)std::stoi(argv[0]);
    return 0;
}

int database::get_user_data_callback(int argc, char **argv, char **azColName)
{
    str_res = {argv[0]};
    int_res  = std::stoll(argv[1]);

    return 0;
}

int database::get_strres_vec_callback(int argc, char **argv, char **azColName)
{
    strres_vec.emplace_back(argv[0]);

    return 0;
}

int database::get_strres_callback(int argc, char **argv, char **azColName)
{
    str_res = {argv[0]};

    return 0;
}

int database::get_chats_callback(int argc, char **argv, char **azColName)
{
    chats.emplace_back(std::stoll(argv[0]), argv[1]);

    return 0;
}