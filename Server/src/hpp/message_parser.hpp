#pragma once 
#include <cstring>
#include <string>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string_view>
#include <utility>
#include <bit>
#include "inet.hpp"
#include "chat_message.hpp"

using idsvec   = std::vector<uint64_t>;
using strvec   = std::vector<std::string>;

struct message_parser
{
    enum message_types
    {
        MESSAGE             = 0,
        LOAD_MESSAGES       = 1,
        LOAD_CHATS          = 2,
        LOAD_PARTICIPANTS   = 3,
        SEARCH_USERS        = 4,
        CREATE_SINGLE_CHAT  = 5,
        CREATE_GROUP        = 6,
        DELETE_CHAT         = 7,
        CHANGE_USER_NAME    = 8,
        CHANGE_PASSWORD     = 9,
        AUTHORIZE_USER      = 10,
        REGISTER_USER       = 11,
        END_LOADING         = 12
    };

    struct parsed_message
    {
        uint8_t type;
        idsvec  idargs;
        strvec  strargs;
    };

    static message_parser::parsed_message parse(chat_message const &msg);

    static chat_message parsed_to_chat_message(message_parser::parsed_message const &msg);

private:

    static std::pair<idsvec, strvec> extract_args(chat_message const &msg, size_t start, int idc, int strc);

    static idsvec extract_ids(chat_message const &msg, size_t start, int idc);

    static strvec extract_strs(chat_message const &msg, size_t start, int strc);

    static uint64_t ntohll(uint64_t hostll);

    static uint64_t htonll(uint64_t networkll);
};