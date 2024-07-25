#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>

#include "IRCClient.hpp"

// Maximum number of users allowed in a channel
const int kMaxChannelUsers = 100;

// Enum for channel permissions
enum ChannelPermission {
    kOperator = 1 << 0,  // Operator permission
    kNormal = 1 << 1     // Normal user permission
};

// Enum for channel mode settings
enum ChannelModeSet {
    kLimit = 1 << 0,    // 'l': Limit the number of users
    kInvite = 1 << 1,   // 'i': Invite-only channel
    kTopic = 1 << 2,    // 't': Only operators can change topic
    kPassword = 1 << 3, // 'k': Channel requires a password
    kDefault = 1 << 4
};

typedef std::map<std::string, ChannelPermission> UserInChannel;
typedef std::vector<std::string> InvitedUsers;

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
    void SetTopic(const std::string& nickname, const std::string& topic);

    // User management
    void AddInvitedUser(const std::string& nickname, const std::string& target_nickname);
    void AddChannelUser(const std::string& nickname, const std::string& target_nickname);

    // Password verification
    bool MatchPassword(const std::string& password) const;

    // Permission management
    void ManageChannelPermission(const std::string& nickname, const std::string& target_nickname, ChannelPermission option);

    // Getters
    std::string GetChannelStartTime() const;
    std::string GetTopic() const;
    std::string GetPassword() const;
	std::vector<std::string> GetMemberNames() const;
	std::string GetTopicEditDate() const;

private:
    IRCChannel();
	std::string SetTime();
    bool CheckChannelMode(ChannelModeSet option) const;
 
    std::vector<std::string> invited_users_;
    std::map<std::string, ChannelPermission> users_in_channel_;

    unsigned int channel_mode_;
    unsigned int channel_limit_;
    std::string channel_name_;
    std::string channel_topic_;
    std::string channel_topic_edit_date_;
    std::string channel_password_;
    std::string start_date_;
    
};

#endif  // IRC_CHANNEL_HPP