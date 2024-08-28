#ifndef IRCREQUESTPARSER_HPP
# define IRCREQUESTPARSER_HPP

#include "IRCTypes.hpp"

namespace IRCRequestParser
{
	// parse raw message
	bool ParseMessage(Buffer& message, IRCCommand& command, IRCParams& params);
	void AddNewLineToBuffer(Buffer& buffer);
	IRCParams SeparateParam(const std::string& param, const std::string& delim);

	// add prefix
	std::string AddChanPrefixToParam(const std::string& param);
	std::string DelChanPrefixFromParam(const std::string& param);

	// convert IRCCommand <-> std::string
	std::string ConvertComToStr(const enum IRCCommand command);
	enum IRCCommand ConvertStrToCom(const std::string& command);
};

#endif
