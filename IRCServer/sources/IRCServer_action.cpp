#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

// void IRCServer::ActionKICK(IRCContext& context)
// {
// 	std::cout << "***************************\n";
// 	for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++) {
// 		std::cout << *it << '/';
// 	}
// 	std::cout << "***************************\n";
// }

void IRCServer::ActionPRIVMSG(IRCContext& context)
{
	std::cout << "***************************\n";
	for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++) {
		std::cout << *it << '/';
	}
	std::cout << "***************************\n";
}