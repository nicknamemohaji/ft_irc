#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCRequestParser.hpp"
#include "IRCTypes.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ErrorSender(IRCContext context, unsigned int errornum){
	# ifdef COMMAND
	std::cout << "Error sender process "  << errornum << std::endl;
	# endif
	context.numericResult = errornum;
	context.client->Send(MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
}

void IRCServer::ActionPART(IRCContext& context){
		//  param size =1 channel left;
			std::cout << " PRAT param size = " << context.params.size() <<std::endl;
		if(!(context.params.size() == 1 || context.params.size() == 2)){
			ErrorSender(context, 461);
			return;
		}
		# ifdef COMMAND
			std::cout << "param 0 =" << context.params[0] <<std::endl;
			std::cout << "param 1 = " << context.params[1] <<std::endl;
		# endif
		std::string reason;
		if(context.params.size() == 2)
			reason = context.params[1];
		IRCParams channel_names =  IRCRequestParser::SeparateParam(context.params[0],",");
		#ifdef COMMAND
		std::cout<< "result of paser channel name = " << std::endl;
		for(unsigned int i = 0; i < channel_names.size(); i++){
			std::cout << "channel name "<< i <<" "<< channel_names[i] << std::endl;
		}
		#endif	
		for(unsigned int i = 0; i < channel_names.size(); i++){
			context.channel = GetChannel(IRCRequestParser::AddChanPrefixToParam(channel_names[i]));
			if(!context.channel){
				#ifdef COMMAND
				//채널없음
				std::cout<< "channel error no channel" << channel_names[i] << std::endl;
				#endif	
				context.stringResult = channel_names[i];
				ErrorSender(context,403);
				continue;
			}
			std::string channel_name = context.channel->GetChannelInfo(kChannelName);
			if(!context.channel->IsInChannel(context.client->GetNickname())){
				//채널에 유저 없음
				ErrorSender(context,442);
				continue;
			}
			context.numericResult = -1;
			context.createSource = true;
			context.stringResult = " PART " + context.channel->GetChannelInfo(kChannelName);
			if(!reason.empty())
				context.stringResult = context.stringResult + " :" + reason;
			SendMessageToChannel(context, SendToAll);
			// sendPartMsg(context.channel->GetMemberNames(),context,*this,reason);
			context.channel->DelChannelUser(context.client->GetNickname()); // 채널에서 유저 제거
			if(context.channel->GetChannelUserSize() == 0)
				DelChannel(channel_name); // 서버에서 채널 제거
			context.client->DelChannel(channel_name); // 유저의 채널리스트에서 채널 제거
		}
}