#ifndef IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_
#define IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_

#include <string>

#include "IRCServer/includes/IRCClient.hpp"
#include "IRCServer/includes/IRCContext.hpp"
#include "IRCServer/includes/IRCTypes.hpp"

namespace IRC_response_creator {
// create full message(source + command + params)

std::string MakeResponse(const IRCContext& context);

// methods for creating error response
// TODO(kyungjle) do i need this??

void ErrorSender(IRCContext context, unsigned int errornum);

// numeric response - RPL

void RPL_NOTOPIC(IRCContext& context);
void RPL_TOPIC(IRCContext& context);
void RPL_TOPICWHOTIME(IRCContext& context);
void RPL_ENDOFNAMES(IRCContext& context);
void RPL_NAMREPLY(IRCContext& context);
void RPL_CREATIONTIME(IRCContext& context);
void RPL_CHANNELMODEIS(IRCContext& context);

// numeric response - ERR

void ERR_NOSUCHSERVER(IRCClient* client, const std::string& server_name,
                      FDSet* pending_fds, const std::string& param);  // 402
void ERR_NONICKNAMEGIVEN(IRCClient* client, const std::string& server_name,
                         FDSet* pending_fds);  // 431
void ERR_ERRONEUSNICKNAME(IRCClient* client, const std::string& server_name,
                          FDSet* pending_fds);  // 432
void ERR_NICKNAMEINUSE(IRCClient* client, const std::string& server_name,
                       FDSet* pending_fds);  // 433
void ERR_NOTREGISTERED(IRCClient* client, const std::string& server_name,
                       FDSet* pending_fds);  // 451
void ERR_NEEDMOREPARAMS(IRCClient* client, const std::string& server_name,
                        FDSet* pending_fds, enum IRCCommand command);  // 461
void ERR_ALREADYREGISTERED(IRCClient* client, const std::string& server_name,
                           FDSet* pending_fds);  // 462
void ERR_PASSWDMISMATCH(IRCClient* client, const std::string& server_name,
                        FDSet* pending_fds);  // 464

};  // namespace IRC_response_creator

#endif  // IRCUTILS_INCLUDES_IRCRESPONSECREATOR_HPP_
