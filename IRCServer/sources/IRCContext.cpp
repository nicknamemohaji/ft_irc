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
