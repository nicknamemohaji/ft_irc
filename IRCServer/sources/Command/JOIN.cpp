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


//RPL_CREATIONTIME (329)
// "<client> <channel> <creationtime>"

void IRCServer::ActionJOIN(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "JOIN command start" << std::endl;
	# endif

	std::stringstream result;
	// pram size is over 2 errr parsing 461;
		//throw IRCError::MissingParams(); // 461
	// channel name vaild check
	if(!(context.params.size() > 0 && context.params.size() < 3)){
		ErrorSender(context,461);
		return;
	}
	std::vector<std::string> channel_names_ = ParserSep(context.params[0] , ",");
	std::vector<std::string> channel_passwords_;
	if(context.params.size() > 1)
		channel_passwords_ = ParserSep(context.params[1] , ",");
	# ifdef COMMAND
		for(unsigned int i = 0; i < context.params.size(); ++i)
		{
			std::cout << "context idx" << i << " " << std::endl;
			std::cout << context.params[i] << std::endl;	
			std::cout << std::endl;
		}
	# endif
	for(unsigned int i = 0; i < channel_names_.size();++i){
		std::string channel_name =AddPrefixToChannelName(channel_names_[i]);
		if(isValidChannelName(channel_name)){
			context.stringResult = channel_name; 
			ErrorSender(context, 476);
			continue;
			// throw IRCError::BadChannelName(); //476
		}
		IRCChannel* channel;
		if(!IsChannelInList(channel_name))
		{
			# ifdef COMMAND
			std::cout << "New channel create " << i  << channel_name <<std::endl;
			# endif
			//make new channel
			if(channel_passwords_.size() >1 && channel_passwords_[i] != "x"){
				# ifdef COMMAND
				std::cout << channel_passwords_[i] <<" passowrd channel create ! " <<std::endl;
				# endif	
				channel = AddChannel(context.client->GetNickname(),channel_name,channel_passwords_[i]);
			}
			else{
				# ifdef COMMAND
				std::cout << "no passowrd channel create ! " <<std::endl;
				# endif	
				channel = AddChannel(context.client->GetNickname(),channel_name,"");
			}
			context.client->AddChannel(channel->GetChannelInfo(kChannelName),channel);
			context.channel = channel;
			# ifdef COMMAND
			std::cout << "New channel create check done;" << i <<std::endl;
			# endif
		}
		else{
			# ifdef COMMAND
			std::cout << "exsit channel join "  << i << std::endl;
			# endif
			//exist channel
			//already in channel
			channel = this->GetChannel(channel_name);
			context.channel = channel;
			if(channel->IsInChannel(context.client->GetNickname()) == true){
				# ifdef COMMAND
				std::cout << "client nickname "  << context.client->GetNickname() << std::endl;
				std::cout << "client in channel "  << channel->IsInChannel(context.client->GetNickname()) << std::endl;
				std::cout << "exsit channel continue "  << i << std::endl;
				# endif
				continue;
			}
			# ifdef COMMAND
			std::cout << "channel RPL START;" << i << "is invite >?   " << channel->IsInvited(context.client->GetNickname()) << "is invitemode ??  " << channel->CheckChannelMode(kInvite)<<std::endl;
			# endif
			if(channel->CheckChannelMode(kInvite)==true && channel->IsInvited(context.client->GetNickname()) == false){
				ErrorSender(context, 473);
				continue;
			}
			# ifdef COMMAND
			std::cout << "after invite check " <<std::endl;
			# endif
			//초대 모드인지, 초대된 유저인지 확인
			if(channel->GetChannelInfo(kChannelPassword) != "")
			{
				//check password and correct password
				if(channel_passwords_.size() < i || channel->GetChannelInfo(kChannelPassword) != channel_passwords_[i])
				{
					# ifdef COMMAND
					std::cout << "password error!!!" << i <<std::endl;
					# endif
					ErrorSender(context, 475);
					continue;
					// throw IRCError::BadChannelKey(); // 475 비밀번호
				}
			}
			//check channel userlimit
			if(channel->channel_limit_ <= channel->GetChannelUserSize()){
				ErrorSender(context, 471);
				continue;
			}
				// throw IRCError::ChannelIsFull(); //471 채널 포화
			//check invite mode and isinvited
			/*
			if(modecheck() && !channel.IsInvited(context->client->GetNickname()))
				throw IRCError::InviteOnly();
			if(!channel.IsInvited(context->client->GetNickname())
				delete invite
			*/
			//channel add at client and channel add client
			context.client->AddChannel(channel->GetChannelInfo(kChannelName),channel);
			channel->AddChannelUser(context.client->GetNickname());
			# ifdef COMMAND
			std::cout << "exsit channel join check done;" << i <<std::endl;
			# endif
		}
		/*
			채널 최대 유저수 확인
			do.
		*/
		channel->DelInvitedUser(context.client->GetNickname());//채널의 초대리스트에서 제거
		context.client->DelInviteChannel(channel_name); // 유저의 초대 채널리스트에서 제거
		// sendJoinMsg(context);
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = " JOIN " + context.channel->GetChannelInfo(kChannelName);
		SendMessageToChannel(context,true);
		if(channel->GetChannelInfo(kTopicInfo) != ""){
			RPL_TOPIC(context);
			RPL_TOPICWHOTIME(context);//RPL_TOPIC 332, RPL_TOPICWHOTIME 333
		}
		//RPL_CHANNELMODEIS 324
		RPL_NAMREPLY(context);//RPL_NAMREPLY 353
		RPL_CREATIONTIME(context); //CREATIONTIME 329
		//send message all user in channel user incomming
		# ifdef COMMAND
		std::cout << "channel RPL done;" << i <<std::endl;
		# endif
	}
	# ifdef COMMAND
	std::cout << "all join process done" << std::endl;
	# endif
	///do join
}

		// result.str("");
		// context.stringResult.clear();
		// result << clientNickname
		// 	<< " :Your host is "<< _serverName << ", running version " << VERSION;
		// context.numericResult = 2;
		// context.stringResult = result.str();
		// context.client->Send(MakeResponse(context));
# ifndef VERSION
# define VERSION "42.42"
# endif

// void IRCServer::ActionJOIN(IRCContext& context)
// {
// 	std::stringstream result;

// 	///do join
// }
