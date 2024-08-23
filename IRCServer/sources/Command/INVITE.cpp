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

void IRCServer::ActionINVITE(IRCContext &context){
	# ifdef COMMAND
	std::cout << "invite command access" << std::endl;
	for(unsigned int i = 0; i < context.params.size(); i++){
		std::cout << context.params[i] << std::endl;
	}
	# endif
	if(context.params.size() != 2 ){
		ErrorSender(context, 461); // 파라미터 에러.
		return;
	}
	std::string user_name = context.params[0];
	std::string channel_name = AddPrefixToChannelName(context.params[1]);
	if(!IsChannelInList(channel_name) || !IsUserInList(user_name)){
		ErrorSender(context, 401);// 채널 없음 | 유저 없음
		return;
	}
	IRCChannel *channel = GetChannel(channel_name);
	context.channel = channel;
	if(!channel->IsInChannel(context.client->GetNickname())){
		ErrorSender(context, 442); //초대자가 채널에 없음.
		return;
	}
	if(channel->IsInChannel(user_name)){
		ErrorSender(context, 443); // 유저가 채널에 이미 있음
		return;
	}
	/*
		초대 제한모드인지, 권한이 있는지 확인 하는 부분
	*/
	context.stringResult  = user_name;
	channel->AddInvitedUser(user_name); //채널의 초대리스트에 추가
	context.client->AddInviteChannel(channel_name); // 유저의 초대채널리스트에 추가
	RPL_INVITING(context);
	context.client = GetClient(user_name);	
	RPL_INVITED(context);
}

// :dan-!d@localhost INVITE Wiz #test    ; dan- has invited Wiz
//                                         to the channel #test