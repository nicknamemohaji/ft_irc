#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCTypes.hpp"
#include "IRCRequestParser.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"
#include "IRCResponseCreator.hpp"


//RPL_CREATIONTIME (329)
// "<client> <channel> <creationtime>"

void IRCServer::ActionJOIN(IRCContext& context)
{
	# ifdef JCOMMAND
	std::cout << "JOIN command start" << std::endl;
	# endif

	std::stringstream result;
	if(!(context.params.size() > 0 && context.params.size() < 3)){
		return IRC_response_creator::ERR_NEEDMOREPARAMS(context.client, server_name_, context.pending_fds, context.command);
	}
	IRCParams channel_names_ = IRC_request_parser::SeparateParam(context.params[0] , ",");
	IRCParams channel_passwords_;
	if(context.params.size() > 1)
		channel_passwords_ = IRC_request_parser::SeparateParam(context.params[1] , ",");
	# ifdef COMMAND
		for(unsigned int i = 0; i < context.params.size(); ++i)
		{
			std::cout << "context idx" << i << " " << std::endl;
			std::cout << context.params[i] << std::endl;
			std::cout << "channel passwd size  = " << channel_passwords_.size() <<  " channel namesize =" << channel_names_.size() << std::endl;
			std::cout << std::endl;
		}
	# endif
	for(unsigned int i = 0; i < channel_names_.size();++i){
		std::string channel_name = IRC_request_parser::AddChanPrefixToParam(channel_names_[i]);
		// channel name vaild check
		if(!IRCChannel::isValidChannelName(channel_name)){
			// throw IRCError::BadChannelName(); //476
			context.stringResult = channel_name; 
			IRC_response_creator::ErrorSender(context, 476);
			continue;
		}
		IRCChannel* channel;
		if(!IsChannelInList(channel_name))
		{
			# ifdef JCOMMAND
			std::cout << "New channel create " << i  << channel_name <<std::endl;
			# endif
			//make new channel
			if(!channel_passwords_.empty() && channel_passwords_.size()-1 > i && channel_passwords_[i] != "x"){
				# ifdef JCOMMAND
				std::cout << channel_passwords_[i] <<" passowrd channel create ! " <<std::endl;
				# endif	
				channel = AddChannel(context.client->GetNickname(),channel_name,channel_passwords_[i]);
			}
			else{
				# ifdef JCOMMAND
				std::cout << "no passowrd channel create ! " <<std::endl;
				# endif	
				channel = AddChannel(context.client->GetNickname(),channel_name,"");
			}
			context.client->AddChannel(channel->GetChannelInfo(kChannelName));
			context.channel = channel;
			# ifdef JCOMMAND
			std::cout << "New channel create check done;" << i <<std::endl;
			# endif
		}
		else{
			# ifdef JCOMMAND
			std::cout << "exsit channel join "  << i << std::endl;
			# endif
			//exist channel
			//already in channel
			channel = this->GetChannel(channel_name);
			context.channel = channel;
			if(channel->IsInChannel(context.client->GetNickname()) == true){
				# ifdef JCOMMAND
				std::cout << "client nickname "  << context.client->GetNickname() << std::endl;
				std::cout << "client in channel "  << channel->IsInChannel(context.client->GetNickname()) << std::endl;
				std::cout << "exsit channel continue "  << i << std::endl;
				# endif
				continue;
			}
			# ifdef JCOMMAND
			std::cout << "channel RPL START;" << i << "is invite >?   " << channel->IsInvited(context.client->GetNickname()) << "is invitemode ??  " << channel->CheckChannelMode(kInvite)<<std::endl;
			# endif
			//초대 모드인지, 초대된 유저인지 확인
			if(channel->CheckChannelMode(kInvite) && !channel->IsInvited(context.client->GetNickname())){
				IRC_response_creator::ErrorSender(context, 473);
				continue;
			}
			# ifdef JCOMMAND
			std::cout << "after invite check " <<std::endl;
			# endif
			//check password and correct password
			if(channel->CheckChannelMode(kPassword))
			{
				if(channel_passwords_.empty() || channel_passwords_.size() < i || channel->GetChannelInfo(kChannelPassword) != channel_passwords_[i])
				{
					# ifdef JCOMMAND
					if(!channel_passwords_.empty())
						std::cout << "password error!!!" << i << "password input = " << channel_passwords_[i] <<std::endl;
					std::cout << "password error!!!" << i << "channel password = " << channel->GetChannelInfo(kChannelPassword) <<std::endl;
					# endif
					// throw IRCError::BadChannelKey(); // 475 비밀번호
					IRC_response_creator::ErrorSender(context, 475);
					continue;
				}
			}
			//check channel userlimit
				# ifdef JCOMMAND
					std::cout << "channel limit !!!" << i << "channel limit = " << channel->GetChannelInfo(kChannelUserLimit) <<std::endl;
					std::cout << "channel limit !!!" << i << "channel user number = " << channel->GetChannelUserSize() <<std::endl;
				# endif
			if(channel->CheckChannelMode(kLimit) && static_cast<unsigned int>(std::atoi(channel->GetChannelInfo(kChannelUserLimit).c_str())) <= channel->GetChannelUserSize()){
				IRC_response_creator::ErrorSender(context, 471); //471 채널 포화
				continue;
			}
			//channel add at client and channel add client
			context.client->AddChannel(channel->GetChannelInfo(kChannelName));
			channel->AddChannelUser(context.client->GetNickname());
			# ifdef JCOMMAND
			std::cout << "exsit channel join check done;" << i <<std::endl;
			# endif
		}
		channel->DelInvitedUser(context.client->GetNickname());//채널의 초대리스트에서 제거
		context.client->DelInvitedChannel(channel_name); // 유저의 초대 채널리스트에서 제거
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = context.channel->GetChannelInfo(kChannelName);
		SendMessageToChannel(kChanSendModeToAll, context);
		if(channel->GetChannelInfo(kTopicInfo) != ""){
			IRC_response_creator::RPL_TOPIC(context);
			IRC_response_creator::RPL_TOPICWHOTIME(context);//RPL_TOPIC 332, RPL_TOPICWHOTIME 333
		}
		IRC_response_creator::RPL_NAMREPLY(context);//RPL_NAMREPLY 353
		# ifdef JCOMMAND
		std::cout << "channel RPL done;" << i <<std::endl;
		# endif
	}
	# ifdef JCOMMAND
	std::cout << "all join process done" << std::endl;
	# endif
}
