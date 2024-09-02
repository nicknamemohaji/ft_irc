#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"

#include <string>
#include <sstream>
#include <deque>

#include "IRCResponseCreator.hpp"

void IRCServer::ActionQUIT(IRCContext& context)
{
	IRCClient* client = context.client;
	std::string clientName = client->GetNickname();

	// prepare response
	context.createSource = true;
	context.numericResult = -1;
	std::stringstream ss;
	ss << ":Quit: ";
	for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++)
		ss << *it << " ";
	context.stringResult = ss.str();
	
	// broadcast leave
	IRCClientJoinedChannels _channels = client->ListChannels();
	for (IRCClientJoinedChannels::iterator _it = _channels.begin(); _it != _channels.end(); _it++)
	{
		std::string _channel_name = _it->first;
		context.channel = GetChannel(_channel_name);
		SendMessageToChannel(kChanSendModeToExceptMe, context);
	}

	// delete from channels
	RmClientFromChanJoined(client);
	RmClientFromChanInvited(client);

	// acknoledgement to client
  context.command = UNKNOWN;
  context.createSource = false;
  context.server = this;
	context.stringResult = "ERROR :Closing connection";
	client->Send(IRC_response_creator::MakeResponse(context));
	context.pending_fds->insert(client->GetFD());
	client->SetStatus(PENDING_QUIT);
}
