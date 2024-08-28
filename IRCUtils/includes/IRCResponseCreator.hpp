#ifndef IRCRESPONSECREATOR_HPP
# define IRCRESPONSECREATOR_HPP

#include  <string>

#include "IRCTypes.hpp"
#include "IRCContext.hpp"

namespace IRCResponseCreator
{
	// create full message(source + command + params)
	std::string MakeResponse(IRCContext& context);
	// methods for creating error response
	// TODO: do i need this??
	void ErrorSender(IRCContext context, unsigned int errornum);
	//RPL
	void RPL_NOTOPIC(IRCContext& context);
	void RPL_TOPIC(IRCContext& context);
	void RPL_TOPICWHOTIME(IRCContext& context);
	void RPL_ENDOFNAMES(IRCContext& context);
	void RPL_NAMREPLY(IRCContext& context);
	void RPL_CREATIONTIME(IRCContext& context);
	void RPL_CHANNELMODEIS(IRCContext& context);
};

#endif