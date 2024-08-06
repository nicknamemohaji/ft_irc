#pragma once

#include "IRCContext.hpp"
#include "IRCServer.hpp"

class IRCRpl{
	public:
		static void RPL_NOTOPIC(IRCContext& context);
		static void RPL_TOPIC(IRCContext& context);
		static void RPL_TOPICWHOTIME(IRCContext& context);
		static void RPL_ENDOFNAMES(IRCContext& context);
		static void RPL_NAMREPLY(IRCContext& context);
		static void RPL_CREATIONTIME(IRCContext& context);
	private:
};