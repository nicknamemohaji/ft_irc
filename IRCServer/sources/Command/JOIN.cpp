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

# ifndef VERSION
# define VERSION "42.42"
# endif
/* <channel>{,<channel>} [<key>{,<key>}]
	#42,#hello key,value 
	#42,#hello key	key for 42
	#newchannel   // make new channel.

*/

void RPL_NOTOPIC(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_NOTOPIC start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :No topic is set";
	context.numericResult = 331;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_NOTOPIC end" << std::endl;
	# endif
}

void RPL_TOPIC(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_TOPIC start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :" << context.channel->GetChannelInfo(kTopicInfo);
	context.numericResult = 332;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_TOPIC end" << std::endl;
	# endif
}

void RPL_TOPICWHOTIME(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_TOPICWHOTIME start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.client->GetNickname() << " " << context.channel->GetChannelInfo(kTopicEditTime);
	context.numericResult = 333;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_TOPICWHOTIME end" << std::endl;
	# endif
}

void RPL_ENDOFNAMES(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_ENDOFNAMES start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :End of /NAMES list";
	context.numericResult = 366;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_ENDOFNAMES end" << std::endl;
	# endif
}

void RPL_NAMREPLY(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_NAMREPLY start" << std::endl;
	# endif
	std::deque<std::string> channel_names = context.channel->GetMemberNames();
	std::string names;
	while(!channel_names.empty())
	{
		for(unsigned int i = 0; i < channel_names.size() || i < 5; i++)
		{
			if(channel_names.empty())
				break;
			names += channel_names.front();
			channel_names.pop_front();
			if(!channel_names.empty())
				names += " ";
		}
		std::stringstream result;
		result.str("");
		context.stringResult.clear();
		result << context.client->GetNickname()
			<< " = "<< context.channel->GetChannelInfo(kChannelName) << " :" << names;
		context.numericResult = 353;
		context.stringResult = result.str();
		context.client->Send(context.server->MakeResponse(context));
		context.FDsPendingWrite.insert(context.client->GetFD());
		names.clear();
	}
	# ifdef COMMAND
	std::cout << "RPL_NAMREPLY end" << std::endl;
	# endif
	RPL_ENDOFNAMES(context);
}

//RPL_CREATIONTIME (329)
// "<client> <channel> <creationtime>"

void IRCServer::ActionJOIN(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "JOIN command start" << std::endl;
	# endif

	std::stringstream result;
	// pram size is over 2 errr parsing 461;
	if(context.params.size() > 2)
		throw IRCError::MissingParams(); // 461
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
		if(isValidChannelName(PaseringMatrix[0][i]))
			throw IRCError::BadChannelName(); //476
	}
	for(unsigned int i = 0; i < PaseringMatrix[0].size();++i){
		IRCChannel* channel;
		if(!IsChannelInList(PaseringMatrix[0][i]))
		{
			# ifdef COMMAND
			std::cout << "New channel create " << i <<std::endl;
			# endif
			//make new channel
			if(PaseringMatrix.size() > 1 && PaseringMatrix[1].size() > i)
				channel = AddChannel(context.client->GetNickname(),PaseringMatrix[0][i],PaseringMatrix[1][i]);
			else
				channel = AddChannel(context.client->GetNickname(),PaseringMatrix[0][i],"");
			context.client->AddChannel(channel->GetChannelInfo(kChannelName),channel);
			# ifdef COMMAND
			std::cout << "New channel create check done;" << i <<std::endl;
			# endif
		}
		else{
			# ifdef COMMAND
			std::cout << "exsit channel create "  << i << std::endl;
			# endif
			//exist channel
			//already in channel
			channel = this->GetChannel(PaseringMatrix[0][i]);
			if(channel->IsInChannel(context.client->GetNickname()) == true){
				# ifdef COMMAND
				std::cout << "client nickname "  << context.client->GetNickname() << std::endl;
				std::cout << "client in channel "  << channel->IsInChannel(context.client->GetNickname()) << std::endl;
				std::cout << "exsit channel continue "  << i << std::endl;
				# endif
				continue;
			}
			if(PaseringMatrix.size() > 1 && PaseringMatrix[1].size() > i)
			{
				//check password and correct password
				if(channel->GetChannelInfo(kChannelPassword) != "" && channel->GetChannelInfo(kChannelPassword) != PaseringMatrix[1][i])
					throw IRCError::BadChannelKey(); // 475 비밀번호
				//check channel userlimit
				if(channel->channel_limit_ <= channel->GetChannelUserSize())
					throw IRCError::ChannelIsFull(); //471 채널 포화
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
			}
			# ifdef COMMAND
			std::cout << "exsit channel create check done;" << i <<std::endl;
			# endif
		}
		# ifdef COMMAND
		std::cout << "channel RPL START;" << i <<std::endl;
		# endif
		context.channel = channel;
		if(channel->GetChannelInfo(kTopicInfo) == "")
			RPL_NOTOPIC(context);//RPL_NOTOPIC 333
		else{
			RPL_TOPIC(context);
			RPL_TOPICWHOTIME(context);//RPL_TOPIC 332, RPL_TOPICWHOTIME 333
		}
		//RPL_CHANNELMODEIS 324
		RPL_NAMREPLY(context);//RPL_NAMREPLY 353
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