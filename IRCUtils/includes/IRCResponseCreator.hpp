#ifndef IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_
#define IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_

#include  <string>

#include "IRCServer/includes/IRCTypes.hpp"
#include "IRCServer/includes/IRCContext.hpp"

namespace IRC_response_creator {
// create full message(source + command + params)
std::string MakeResponse(const IRCContext& context);
// methods for creating error response
// TODO(kyungjle) do i need this??
void ErrorSender(IRCContext context, unsigned int errornum);
// RPL
void RPL_NOTOPIC(IRCContext& context);
void RPL_TOPIC(IRCContext& context);
void RPL_TOPICWHOTIME(IRCContext& context);
void RPL_ENDOFNAMES(IRCContext& context);
void RPL_NAMREPLY(IRCContext& context);
void RPL_CREATIONTIME(IRCContext& context);
void RPL_CHANNELMODEIS(IRCContext& context);
};  // namespace IRC_response_creator

#endif  // IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_
