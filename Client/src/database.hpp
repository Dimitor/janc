#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <cstring>
#include <tuple>
#include <utility>
#include <ctime>
#include <sqlite3.h>

#include "chat_item.hpp"
#include "message_item.hpp"
#include "ncurses.h"

using strcref = std::string const &;
using idtype  = uint64_t;

struct database
{
    database(std::string &&dbname);

    std::vector<message_item> get_messages(idtype chat_id, size_t lborder, size_t rborder);

    std::string get_message_prefix(idtype msg_id);

    int get_message_quantity(idtype chat_id);

    std::vector<std::string> get_chat_participants(idtype chat_id);

    bool chat_exists(idtype chat_id);

    std::vector<chat_item> get_chats();

    int save_message(idtype msg_id, strcref content, idtype chat_id, idtype participant_id, std::time_t creation_time);

    int clear_messages(idtype chat_id);

    int add_chat(idtype chat_id, strcref chat_name);

    int add_chat_participant(idtype chat_id, idtype participant_id);

    int add_participant(idtype participant_id, strcref participant_name);

    int delete_chat(idtype chat_id);

    std::tuple<std::string, size_t> get_user_data();

    bool user_exists();

    std::string get_chat_name(idtype chat_id);

    int update_user_name(strcref new_name);

    int update_user_password_len(int password_len);

    int insert_user_data(strcref username, int password_len);

    int clear_db(bool with_user = true);

    ~database() { sqlite3_close(conn); }

private:

    static int default_callback_static(void *instance, int argc, char **argv, char **azColName) { return 0; }

    static int int_res_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_messages_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_message_prefix_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int boolres_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_user_data_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_strres_vec_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_strres_callback_static(void *instance, int argc, char **argv, char **azColName);

    static int get_chats_callback_static(void *instance, int argc, char **argv, char **azColName);

    ////////////////////////////////////////////////////////////////////////////////////////

    int int_res_callback(int argc, char **argv, char **azColName);

    int get_messages_callback(int argc, char **argv, char **azColName);

    int get_message_prefix_callback(int argc, char **argv, char **azColName);

    int boolres_callback(int argc, char **argv, char **azColName);

    int get_user_data_callback(int argc, char **argv, char **azColName);

    int get_strres_vec_callback(int argc, char **argv, char **azColName);

    int get_strres_callback(int argc, char **argv, char **azColName);

    int get_chats_callback(int argc, char **argv, char **azColName);

    sqlite3 *conn;
    sqlite3_stmt *stmt;
    char *err_msg;
    std::vector<message_item> messages;
    std::vector<chat_item>    chats;
    std::vector<std::string>  strres_vec;
    std::vector<idtype>       intres_vec;
    std::string msg_prefix;
    idtype      int_res;
    std::string str_res;
    bool boolres;
};