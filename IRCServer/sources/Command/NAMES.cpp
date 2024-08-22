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

void IRCServer::ActionNAMES(IRCContext& context){
	//param agument too lage!
	StringMatrix PaseringMatrix = parseStringMatrix(context.params);
	IRCChannel *channel = 0;
	# ifdef COMMAND
		std::cout << "param size = " << context.params.size() <<std::endl;
		std::cout << "matrix size = " << PaseringMatrix.size() <<std::endl;
	# endif
	//파싱오류 많은 파라미터
	if(context.params.size() != 1 || PaseringMatrix.size() != 1){
		// throw IRCError::MissingParams(); // 461
		ErrorSender(context, 461);
		return;
	}
	for(unsigned int i = 0; i < PaseringMatrix.size(); ++i){
	if(PaseringMatrix[0][i] == "")
		continue;
	std::string channel_name = AddPrefixToChannelName(PaseringMatrix[0][i]);
	# ifdef COMMAND
		std::cout << "channel name " << channel_name <<std::endl;
	# endif
	if(this->isValidChannelName(channel_name)){
		// throw IRCError::BadChannelName(); //476
		context.stringResult = channel_name;
		ErrorSender(context, 476);
		return;
	}
	channel = this->GetChannel(channel_name);
	# ifdef COMMAND
		std::cout << "channel = " << !channel <<std::endl;
	# endif
	if(!channel)
	{
		context.stringResult = channel_name;{
		// throw IRCError::NoSuchChannel(); //403
		ErrorSender(context, 403);
		return;
		}
	}
	context.channel = channel;
	if(!channel->IsInChannel(context.client->GetNickname())){
		this->RPL_ENDOFNAMES(context);
		# ifdef COMMAND
		std::cout << "channel name command only endof name rpl " <<std::endl;
		# endif
		continue;
	}
	# ifdef COMMAND
		std::cout << "channel name command do well process " << channel->IsInChannel(context.client->GetNickname())<<std::endl;
	# endif
	this->RPL_NAMREPLY(context);
	}
}
/*
채널 존재 및 이름 유효성 확인 (오류 시 RPL_ENDOFNAMES)
채널 비밀번호 모드 확인 (비밀번호 모드일 경우 RPL_ENDOFNAMES)
사용자가 채널에 가입되어 있지 않으면 해당 채널 무시
채널 사용자 목록 전송
*/