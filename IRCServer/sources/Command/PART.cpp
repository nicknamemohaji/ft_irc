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

void IRCServer::ErrorSender(IRCContext context, unsigned int errornum){
	context.numericResult = errornum;
	context.client->Send(this->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
}

void IRCServer::ActionPART(IRCContext& context){
	# ifdef COMMAND
		std::cout << "param size = " << context.params.size() <<std::endl;
		std::cout << "param 0 =" << context.params[0] <<std::endl;
		std::cout << "param 1 = " << context.params[1] <<std::endl;
	# endif
		//  param size =1 channel left;
		std::string reason;
		if(context.params.size() == 2)
			reason = context.params[1];
		std::vector<std::string>channel_name_arry =  PaserSep(context.params[0],",");
		#ifdef COMMAND
		std::cout<< "result of paser channel name = " << std::endl;
		for(unsigned int i = 0; i < channel_name_arry.size(); i++){
			std::cout << "channel name "<< i <<" "<< channel_name_arry[i] << std::endl;
		}
		#endif	
		for(unsigned int i = 0; i)
			context.channel = GetChannel(context.params[0]);
			context.stringResult = context.params[0];
			ErrorSender(context,403);
			if(context.channel)
			{
				//error channel
			#ifdef COMMAND
				std::cout<< "channel error no channel" << context.params[0] << std::endl;
			#endif	
			}
			#ifdef COMMAND
			std::cout<< "param size ==1 left " << context.params[0] << std::endl;
			#endif
}