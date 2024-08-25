#ifndef IRCCHANNEL_HPP
#define IRCCHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>
#include <sstream>
#include <deque>
#include <iostream>

#include "IRCClient.hpp"

// Maximum number of users allowed in a channel
const int kMaxChannelUsers = 100;

// Enum for channel permissions
enum ChannelPermission {
    kOperator = 0,  // Operator permission
    kNormal = 1  // Normal user permission
};

enum ChannelMode {
	kInvite = 0,
    kTopic = 1,
    kPassword = 2,
    kLimit = 3
};

enum ChannelInfo {
    kChannelPassword = 0,    // channel password idx
    kTopicEditUser = 1,   // channel topic edit user idx
    kTopicInfo = 2,    // channel topic info idx
    kTopicEditTime = 3, // channel topic edit time idx
    kChannelDate = 4, // channel making date idx
    kChannelName = 5, //channel name idx
	kChannelUserLimit = 6
};
typedef std::map<std::string, ChannelPermission> UserInChannel;
typedef std::vector<std::string> InvitedUsers;
typedef std::deque<std::string> ChannelUsers;

class IRCChannel {
public:
    IRCChannel(const std::string& nickname, const std::string& channel_name);
    IRCChannel(const std::string& nickname, const std::string& channel_name, const std::string& passwd);
    ~IRCChannel();

    // User authorization and presence checks
    bool IsUserAuthorized(const std::string& nickname, ChannelPermission option);
    bool IsInChannel(const std::string& nickname) const;
    bool IsInvited(const std::string& nickname) const;

    // Setters
    void SetUserAuthorization(const std::string& nickname, ChannelPermission option);
    void SetChannelInfo(ChannelInfo idx, const std::string& str);
    void SetTopic(const std::string& nickname, const std::string& topic);

    // User management
    void AddInvitedUser(const std::string& target_nickname);
    void DelInvitedUser(const std::string& target_nickname);
    void AddChannelUser(const std::string& nickname);
    void DelChannelUser(const std::string& nickname);

    // Password verification
    bool MatchPassword(const std::string& password) const;

    // Permission management
    void ManageChannelPermission(const std::string& target_nickname, ChannelPermission option);

    // Getters
    std::string GetChannelInfo(ChannelInfo idx) const;
	std::deque<std::string> GetMemberNames() const;
	std::deque<std::string> GetChannelUsersWithPrefixes() const;
    unsigned int GetChannelUserSize() const;

    //Translater
    std::string itostr(long long time) const;
    
	//MODE
    bool CheckChannelMode(ChannelMode option) const;
	void SetChannelMode(ChannelMode option, bool flag);
	std::string GetChannelMode() const;

private:
    IRCChannel();
 
    std::vector<std::string> invited_users_;
    std::map<std::string, ChannelPermission> users_in_channel_;

    unsigned int channel_mode_;
    std::string channel_info_arr_ [7];
	bool channel_mode_status_ [4];
};

#endif  // IRC_CHANNEL_HPP