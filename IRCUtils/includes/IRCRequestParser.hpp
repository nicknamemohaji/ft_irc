#ifndef IRCUTILS_INCLUDES_IRCREQUESTPARSER_HPP_
#define IRCUTILS_INCLUDES_IRCREQUESTPARSER_HPP_

#include <string>

#include "IRCServer/includes/IRCTypes.hpp"

namespace IRC_request_parser {
// parse raw message
bool ParseMessage(Buffer* message, IRCCommand* command, IRCParams* params);
void AddNewLineToBuffer(Buffer* buffer);
IRCParams SeparateParam(const std::string& param, const std::string& delim);

// add prefix
std::string AddChanPrefixToParam(const std::string& param);
std::string DelChanPrefixFromParam(const std::string& param);

// convert IRCCommand <-> std::string
std::string ConvertComToStr(const enum IRCCommand command);
enum IRCCommand ConvertStrToCom(const std::string& command);
};  // namespace IRC_request_parser

#endif  // IRCUTILS_INCLUDES_IRCREQUESTPARSER_HPP_
