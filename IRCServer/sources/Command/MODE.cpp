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

void IRCServer::ActionMODE(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "*MODE command start*" << std::endl;
	std::cout << "context.params.size = " << context.params.size() << std::endl;
	# endif
	// std::stringstream result;
	// // pram size is over 2 errr parsing 461;
	// if(context.params.size() > 2)
	// 	throw IRCError::MissingParams(); // 461
	// // channel name vaild check
	StringMatrix PaseringMatrix = parseStringMatrix(context.params);
	# ifdef COMMAND
		std::cout << "paser idx 0 size = " << PaseringMatrix[0].size() << std::endl;
		for(unsigned int i = 0; i < PaseringMatrix.size(); ++i)
		{
			std::cout << "idx" << i << " :";
			for(unsigned int j = 0; j < PaseringMatrix[i].size(); ++j)
				std::cout << PaseringMatrix[i][j] << std::endl;
			std::cout << std::endl;
		}
		std::cout << "channal_name : " << PaseringMatrix[0][0] << std::endl;
	# endif

	std::string mode_result;
	std::vector<std::string> add_result;
	IRCChannel *channel = this->GetChannel(PaseringMatrix[0][0]);
	int idx = 2;
	int flag;
	for(unsigned int i = 0; i < PaseringMatrix[1].size(); i++) {
		if(PaseringMatrix[1][i] == "+") {
			mode_result += "+";
			flag = 1; //true
		}
		else if(PaseringMatrix[1][i] == "-") {
			mode_result += "-";
			flag = 0; //false
		}
		else {
			if(PaseringMatrix[1][i] == "i") {
				if((!channel->CheckChannelMode(kInvite) && flag) || (channel->CheckChannelMode(kInvite) && !flag)){
					channel->SetInvite(!flag);
					mode_result += "i";
				}
			}
			else if(PaseringMatrix[1][i] == "t") {
				if((!channel->CheckChannelMode(kTopic) && flag) || (channel->CheckChannelMode(kTopic) && !flag)) {
					channel->SetTopic(!flag);
					mode_result += "t";
				}
			}
			else if(PaseringMatrix[1][i] == "k") {
				if(!channel->CheckChannelMode(kPassword) && flag) {
					channel->SetPassword(PaseringMatrix[idx][0]);
					mode_result += "k";
					add_result.push_back(PaseringMatrix[idx][0]);
					idx++;
				}
				else if(channel->CheckChannelMode(kPassword) && !flag) {
					channel->SetPassword("");
					mode_result += "k";
					add_result.push_back("*");
					idx++;
				}
			}
			else if(PaseringMatrix[1][i] == "o") {
				
			}
			else if(PaseringMatrix[1][i] == "l") {
				if(!channel->CheckChannelMode(kLimit) && flag) {
					int limit = strtod(PaseringMatrix[idx][0].c_str(), nullptr);
					channel->SetLimit(limit);
					mode_result += "l";
					add_result.push_back(std::to_string(limit));
					idx++;
				}
				else if(channel->CheckChannelMode(kLimit) && !flag) {
					channel->SetLimit(-1);
					mode_result += "l";
				}
			}
			else{
			}
		}
	}
}