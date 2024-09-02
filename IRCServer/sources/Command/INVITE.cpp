#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCTypes.hpp"
#include "IRCContext.hpp"
#include "IRCRequestParser.hpp"
#include "IRCResponseCreator.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionINVITE(IRCContext &context){
	if(context.params.size() != 2 ){
		IRC_response_creator::ErrorSender(context, 461); // 파라미터 에러.
		return;
	}
	std::string _nickname = context.params[0];
	std::string _channel_name = IRC_request_parser::AddChanPrefixToParam(context.params[1]);
	if(!IsChannelInList(_channel_name) || !IsUserInList(_nickname)){
		IRC_response_creator::ErrorSender(context, 401);// 채널 없음 | 유저 없음
		return;
	}

	IRCChannel *channel = GetChannel(_channel_name);
	context.channel = channel;
	if(!channel->IsInChannel(context.client->GetNickname())){
		IRC_response_creator::ErrorSender(context, 442); //초대자가 채널에 없음.
		return;
	}
	if(channel->IsInChannel(_nickname)){
		context.stringResult = _nickname;
		IRC_response_creator::ErrorSender(context, 443); // 유저가 채널에 이미 있음
		return;
	}
	if(channel->CheckChannelMode(kInvite) && !channel->IsUserAuthorized(context.client->GetNickname(), kOperator)){
		IRC_response_creator::ErrorSender(context, 482);
		return;
	}
	context.stringResult  = _nickname;
	channel->AddInvitedUser(_nickname); //채널의 초대리스트에 추가
	// send RPL_INVITING
	{
		std::stringstream result;
		result << context.client->GetNickname() << " " << _nickname << " "<< _channel_name;
		context.numericResult = 341;
		context.createSource = false;
		context.stringResult = result.str();
		context.client->Send(IRC_response_creator::MakeResponse(context));
		context.pending_fds->insert(context.client->GetFD());
	}
	// send INVITE message to invitee
	{
		IRCClient* _invitee = GetClient(_nickname);
		std::stringstream result;
		context.createSource = true;
		result << _nickname << " "<< _channel_name;
		context.numericResult = -1;
		context.stringResult = result.str();
		_invitee->Send(IRC_response_creator::MakeResponse(context));
		context.pending_fds->insert(_invitee->GetFD());
	}
	context.client->AddInviteChannel(_channel_name); // 유저의 초대채널리스트에 추가
}

// :dan-!d@localhost INVITE Wiz #test    ; dan- has invited Wiz
//                                         to the channel #test