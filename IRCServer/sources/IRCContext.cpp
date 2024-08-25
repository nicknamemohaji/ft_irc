#include <string>
#include <deque>
#include <iostream>
#include <vector>

#include "IRCContext.hpp"
#include "IRCErrors.hpp"

IRCContext::IRCContext(std::set<int>& FDset):
	FDsPendingWrite(FDset)
{
	command = UNKNOWN;
	params.clear();
	rawMessage.clear();
	createSource = false;

	server = NULL;
	channel = NULL;
	client = NULL;

	numericResult = -1;
	stringResult.clear();
}

std::ostream& operator<< (std::ostream& ostream, const IRCContext& context)
{
	ostream << "\n--- IRCContext Dump ---\n";
	ostream << "command: [" << context.command << "]" << std::endl;
	ostream << "params: " << std::endl;
	for (std::deque<std::string>::const_iterator it = context.params.begin(); it != context.params.end(); it++)
		ostream << "-- [" << *it << "]" << std::endl;
	ostream << "numericResult: " << context.numericResult << std::endl;
	ostream << "stringResult: " << context.stringResult << std::endl;

	return ostream;
}

enum IRCCommand IRCContext::ConvertStrToCommand(const std::string& command)
{
	if (command == "CAP")
		return CAP;
	else if (command == "PASS")
		return PASS;
	else if (command == "USER")
		return USER;
	else if (command == "NICK")
		return NICK;
	else if (command == "MOTD")
		return MOTD;
	else if (command == "motd")	// WTF
		return MOTD;
	else if (command == "QUIT")
		return QUIT;
	else if (command == "JOIN")
		return JOIN;
	else if (command == "NAMES")
		return NAMES;
	else if (command == "MODE")
		return MODE;
	else if (command == "PART")
		return PART;
	else if (command == "TOPIC")
		return TOPIC;
	else if (command == "KICK")
		return KICK;
	else if (command == "PRIVMSG")
		return PRIVMSG;
	else if (command == "INVITE")
		return INVITE;
	else
		throw IRCError::UnknownCommand();
}

std::string IRCContext::ConvertCommandToStr(enum IRCCommand command)
{
	if (command == CAP)
		return "CAP";
	else if (command == PASS)
		return "PASS";
	else if (command == USER)
		return "USER";
	else if (command == NICK)
		return "NICK";
	else if (command == QUIT)
		return "QUIT";
	else if (command == MOTD)
		return "MOTD";
	else if (command == JOIN)
		return "JOIN";
	else if (command == NAMES)
		return "NAMES";
	else if (command == MODE)
		return "MODE";
	else if (command == PART)
		return "PART";
	else if (command == TOPIC)
		return "TOPIC";
	else if (command == KICK)
		return "KICK";
	else if (command == PRIVMSG)
		return "PRIVMSG";
	else if (command == INVITE)
		return "INVITE";
	else
		throw IRCError::UnknownCommand();
}
