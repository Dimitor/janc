#include "message_parser.hpp"

message_parser::parsed_message message_parser::parse(chat_message const &msg)
{
    using message_types = message_parser::message_types;

    message_parser::parsed_message res;
    res.type = (uint8_t)msg.body()[0];
    switch (res.type)
    {
        case message_types::MESSAGE:
        case message_types::SEARCH_USERS:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 1, 1);
            
            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::LOAD_MESSAGES:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 3, 0);
            
            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::LOAD_CHATS:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 0, 0);

            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::LOAD_PARTICIPANTS:
        case message_types::CREATE_SINGLE_CHAT:
        case message_types::DELETE_CHAT:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 1, 0);

            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::CREATE_GROUP:
        {
            uint64_t quantity;
            std::memcpy(&quantity, msg.body() + 1, sizeof(uint64_t));
            quantity = ntohll(quantity);
            
            auto [idargs, strargs] = extract_args(msg, 1 + sizeof(uint64_t), quantity, 1);
            
            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::CHANGE_USER_NAME:
        case message_types::CHANGE_PASSWORD:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 0, 1);
            
            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        case message_types::AUTHORIZE_USER:
        case message_types::REGISTER_USER:
        {
            auto [idargs, strargs] = extract_args(msg, 1, 0, 2);
            
            res.idargs  = idargs;
            res.strargs = strargs;
        }
        break;
        default:
        {
            printf("UNKNOWN MESSAGE TYPE \"%d\"!!!", res.type);
        }
    }
    return res;
}

std::pair<idsvec, strvec> message_parser::extract_args(chat_message const &msg, size_t start, int idc, int strc)
{
    return std::make_pair(extract_ids(msg, start, idc),
                          extract_strs(msg, start + idc * 8, strc));
}

idsvec message_parser::extract_ids(chat_message const &msg, size_t start, int idc)
{
    idsvec res(idc);

    for (size_t i = 0; i < idc; ++i)
    {
        uint64_t id;
        std::memcpy(&id, msg.body() + start + i * 8, 8);
        res[i] = ntohll(id);
    }

    return res;
}

strvec message_parser::extract_strs(chat_message const &msg, size_t start, int strc)
{
    strvec res;
    res.reserve(strc);

    if (strc == 0) return res;

    size_t pos = start;
    for (; pos < msg.body_length() && res.size() < strc - 1; ++pos)
    {
        if (msg.body()[pos] == ' ')
        {
            res.emplace_back(msg.body() + start, pos - start);
            start = pos + 1;
        }
    }
    res.emplace_back(msg.body() + start, msg.body_length() - start);

    return res;
}

uint64_t message_parser::ntohll(uint64_t hostll)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        hostll = (((uint64_t)ntohl(hostll & 0xFFFFFFFFUL)) << 32) | ntohl((uint32_t)(hostll >> 32));
    }
    return hostll;
}

uint64_t message_parser::htonll(uint64_t networkll)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        networkll = (((uint64_t)htonl((networkll) & 0xFFFFFFFFUL)) << 32) | htonl((uint32_t)((networkll) >> 32));
    }
    return networkll;
}

chat_message message_parser::parsed_to_chat_message(message_parser::parsed_message const &msg)
{
    chat_message message(580000);

    msg_len_t body_len = msg.idargs.size() * 8 + 1;

    message.body()[0] = msg.type;

    for (size_t i = 0; i < msg.idargs.size(); ++i)
    {
        std::memcpy((message.body() + 1 + i * 8), (msg.idargs.data() + i), 8);
    }

    if (!msg.strargs.empty())
    {
        std::string sending_str;
        sending_str.reserve(msg.strargs.size() * 32);

        for (size_t i = 0; i < msg.strargs.size() - 1; ++i)
        {
            sending_str.append(msg.strargs[i]);
            sending_str.push_back(' ');
        }

        sending_str.append(msg.strargs.back());

        memcpy((message.body() + body_len), sending_str.c_str(), sending_str.size());
        body_len += sending_str.length();
    }

    message.body_length(body_len);
    message.encode_header();

    return message;
}