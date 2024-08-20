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


void sendJoinMsg(std::deque<std::string> user_in_channel, IRCContext context, IRCServer& server)
{	
	std::stringstream result;
	IRCClient *user_to_msg;
	std::string user_name = context.client->GetNickname();
	# ifdef COMMAND
	std::cout << "JOIN msg to channel size = "  << user_in_channel.size() << std::endl;
	# endif
	for(unsigned int i = 0; i < user_in_channel.size(); ++i){
		# ifdef COMMAND
		std::cout << "JOIN msg to channel user = "  << user_in_channel[i] << std::endl;
		# endif
		std::string ret;
		result.str("");
		// result <<":"<< user_name << "!" << user_name << "@ft_irc.com"
		result <<":"<< user_name
		<< " JOIN :" << context.channel->GetChannelInfo(kChannelName) << "\r\n";
		ret = result.str();
		user_to_msg = server.GetClient(user_in_channel[i]);
		if(!user_to_msg)
			continue;
	# ifdef COMMAND
		// std::cout << "Sucesse sned JOIN msg to channel, to " << user_to_msg->GetNickname()  << "the fd is" << user_to_msg->GetFD() << std::endl;
		std::cout << "msg" << std::endl;
		std::cout << ret << std::endl;
	# endif	
		context.client = user_to_msg;
		context.client->Send(ret);
		context.FDsPendingWrite.insert(context.client->GetFD());
	}
	# ifdef COMMAND
		std::cout << "JOIN msg to channel end "  << std::endl;
	# endif
}

void IRCServer::ActionJOIN(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "JOIN command start" << std::endl;
	# endif

	std::stringstream result;
	// pram size is over 2 errr parsing 461;
	if(context.params.size() > 2)
		ErrorSender(context,461);
		//throw IRCError::MissingParams(); // 461
	// channel name vaild check
	StringMatrix PaseringMatrix = parseStringMatrix(context.params);
	# ifdef COMMAND
		std::cout << "paser idx 0 size = " << PaseringMatrix[0].size() << std::endl;
		for(unsigned int i = 0; i < PaseringMatrix.size(); ++i)
		{
			std::cout << "idx" << i << " " << std::endl;
			for(unsigned int j = 0; j < PaseringMatrix[i].size(); ++j)
				std::cout << PaseringMatrix[i][j] << std::endl;	
			std::cout << std::endl;
		}
	# endif
	for(unsigned int i = 0; i < PaseringMatrix[0].size();++i)
	{
		if(isValidChannelName(PaseringMatrix[0][i])){
			context.stringResult = PaseringMatrix[0][i]; 
			ErrorSender(context, 476);
			// throw IRCError::BadChannelName(); //476
		}
	}
	for(unsigned int i = 0; i < PaseringMatrix[0].size();++i){
		IRCChannel* channel;
		if(!IsChannelInList(PaseringMatrix[0][i]))
		{
			# ifdef COMMAND
			std::cout << "New channel create " << i <<std::endl;
			# endif
			//make new channel
			if(PaseringMatrix.size() > 1 && PaseringMatrix[1].size() >= i){
			# ifdef COMMAND
			std::cout << PaseringMatrix[1][i] <<"passowrd channel create ! " <<std::endl;
			# endif	
				channel = AddChannel(context.client->GetNickname(),PaseringMatrix[0][i],PaseringMatrix[1][i]);
			}
			else{
			# ifdef COMMAND
			std::cout << "no passowrd channel create ! " <<std::endl;
			# endif	
				channel = AddChannel(context.client->GetNickname(),PaseringMatrix[0][i],"");
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
			channel = this->GetChannel(PaseringMatrix[0][i]);
			context.channel = channel;
			if(channel->IsInChannel(context.client->GetNickname()) == true){
				# ifdef COMMAND
				std::cout << "client nickname "  << context.client->GetNickname() << std::endl;
				std::cout << "client in channel "  << channel->IsInChannel(context.client->GetNickname()) << std::endl;
				std::cout << "exsit channel continue "  << i << std::endl;
				# endif
				continue;
			}
			if(channel->GetChannelInfo(kChannelPassword) != "")
			{
				//check password and correct password
				if(PaseringMatrix.size() < 2 ||  PaseringMatrix[1].size() < i || channel->GetChannelInfo(kChannelPassword) != PaseringMatrix[1][i])
				{
					# ifdef COMMAND
					std::cout << "password error!!!" << i <<std::endl;
					# endif
					ErrorSender(context, 475);
					// throw IRCError::BadChannelKey(); // 475 비밀번호
				}
			}
			//check channel userlimit
			if(channel->channel_limit_ <= channel->GetChannelUserSize())
				ErrorSender(context, 471);
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
		# ifdef COMMAND
		std::cout << "channel RPL START;" << i <<std::endl;
		# endif
		sendJoinMsg(context.channel->GetMemberNames(),context, *this);
		if(channel->GetChannelInfo(kTopicInfo) == "")
			this->RPL_NOTOPIC(context);//RPL_NOTOPIC 333
		else{
			this->RPL_TOPIC(context);
			this->RPL_TOPICWHOTIME(context);//RPL_TOPIC 332, RPL_TOPICWHOTIME 333
		// IRCServer::RPL_JOIN(context);//join alert
		}
		//RPL_CHANNELMODEIS 324
		this->RPL_NAMREPLY(context);//RPL_NAMREPLY 353
		this->RPL_CREATIONTIME(context); //CREATIONTIME 329
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