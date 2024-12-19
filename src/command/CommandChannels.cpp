# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"
#include "ChannelUtils.hpp"

void Command::handleJoin(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string channelName = trim(args);
	if (channelName.empty()) {
		std::string error = "Error: No channel name provided.\n";
		client.sendMessage(error);
		return;
	}
	if(channelName[0] != '#'){
		std::string error = "Error: Name of the channel must start with '#'.\n";
		client.sendMessage(error);
		return;
	}
	Channel* targetChannel = NULL;
	bool found = false;
	std::map<std::string, Channel*>::iterator it = channels.begin();
	for (; it != channels.end(); ++it) {
		if (it->first == channelName) {
			found = true;
			targetChannel = it->second;
		}
	}

	if (!found) {
		targetChannel = _server.getOrCreateChannel(channelName);
		targetChannel->addMember(&client);
		targetChannel->addOperator(&client);
		std::cout << channelName << " was created!.\n";
		return;
	} else {
		// Check if the client is in channel;
		for (std::vector<Client*>::const_iterator it = targetChannel->getMembers().begin();
			it != targetChannel->getMembers().end(); ++it) {
			if (*it == &client) {
				std::string error = "Already in the channel.\n";
				client.sendMessage(error);
				return;
			}
		}
		// check if chaennel is full
		if (targetChannel->getMembers().size() >= static_cast<size_t>(targetChannel->getLimit())) {
			std::string error = "Channel is full. Unable to join.\n";
			client.sendMessage(error);
			return;
		}

		// check if invitation required
		if (targetChannel->getInviteStatus()) {
			bool invited = false;
			for (std::vector<Client*>::const_iterator it = targetChannel->getAllowedPeople().begin();
				 it != targetChannel->getAllowedPeople().end(); ++it) {
				if (*it == &client) {
					invited = true;
					break;
				}
			}
			if (!invited) {
				std::string error = "This is a private channel, you need an invitation.\n";
				client.sendMessage(error);
				return;
			}
		}
		if (!targetChannel->getPassword().empty()) {
			std::string response = "Private channel, requires a password.\n";
			client.sendMessage(response);
			// wait for password
			char passBuff[4096];
			memset(passBuff, 0, sizeof(passBuff));
			int passBytesRecv = 0;
			// Loop to wait for password input
			while (true) {
				passBytesRecv = recv(client.getFd(), passBuff, sizeof(passBuff) - 1, 0);
				if (passBytesRecv > 0) {
					std::string pass(passBuff, passBytesRecv);
					pass.erase(pass.find_last_not_of(" \n\r\t") + 1);
					if (targetChannel->getPassword() == pass) {
						targetChannel->addMember(&client);
						std::string message = "Joined channel: " + targetChannel->getName() + ".\n";
						client.sendMessage(message);
						std::cout << client.getNick() << " has joined the channel: " << targetChannel->getName() << std::endl;
						if (!targetChannel->getTopic().empty())
							targetChannel->broadcastTopic(&client);
						return;
					} else {
						std::string error = "Incorrect password.\n";
						client.sendMessage(error);
						return;
					}
				}
			}
		}
	}
	// Join the channel
	targetChannel->addMember(&client);
	std::string message = "Joined channel: " + targetChannel->getName() + ".\n";
	client.sendMessage(message);
	std::cout << client.getNick() << " has joined the channel: " << targetChannel->getName() << std::endl;
	if (!targetChannel->getTopic().empty()) {
		targetChannel->broadcastTopic(&client);
	}
}


void	Command::handleKick(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string target;
	stream >> target;

	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	if (target.empty()) {
		std::string error = "You need to choose a client to kick.\n";
		client.sendMessage(error);
		return;
	}

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isOperator = isOperatorFct(targetChannel,client);
	if (isOperator){
		bool found = false;
		for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
			if (targetChannel->getMembers()[i]->getNick() == target) {
				found = true;
				if(target == client.getNick()){
					std::string error = "You can t kick yourself dummy :))!. Use PART <#channel name> to exit channel.";
					client.sendMessage(error);
					return;
				}
				std::string response = "You got kicked from the channel:" + channelName + "\n";
				send(targetChannel->getMembers()[i]->getFd(), response.c_str(), response.size() + 1, 0);
				std::cout << targetChannel->getMembers()[i]->getNick() << " got kicked from: " << targetChannel->getName()<<std::endl;
				targetChannel->removeMember(targetChannel->getMembers()[i]);
				break;
			}
		}
		if (!found){
			std::string error = "User not found in the channel " + channelName + ".\n";
			client.sendMessage(error);
		}
	}
	else {
		std::string error = "You 're not an operator in " + channelName +  "\n";
		client.sendMessage(error);
	}
}

void	Command::handlePart(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isOperator = isOperatorFct(targetChannel,client);
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			targetChannel->removeMember(&client);
			if(isOperator)
			{
				targetChannel->removeOperator(&client);

			}
			std::string response = "Exited channel:" +targetChannel->getName() + "\n";
			client.sendMessage(response);
			std::cout << client.getNick() << " exited from: " << targetChannel->getName()<<std::endl;
			break;
		}
	}
	if (!found){
		std::string error = "User are not part of that channell.\n";
		client.sendMessage(error);
		return ;
	}
	if(targetChannel->getMembers().size() == 0){
		std::cout << targetChannel->getName() << " was removed!\n";
		_server.removeChannel(targetChannel->getName());
		return;
	}
	if (targetChannel->getOperators().size() == 0 && targetChannel->getMembers().size() > 0) {
        Client* oldestMember = targetChannel->getMembers().front();
        targetChannel->addOperator(oldestMember);
        std::string promotionMessage = "You have been promoted to operator in channel: " + targetChannel->getName() + "\n";
        oldestMember->sendMessage(promotionMessage);
        std::cout << oldestMember->getNick() << " promoted to operator in channel: " << targetChannel->getName() << std::endl;
    }

}


void	Command::handleTopic(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string topic;
	stream >> topic;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	if(topic.empty()){
		std::string error = "topic can t be empty.\n";
		client.sendMessage(error);
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isMember = isMemberFct(targetChannel,client,channelName, channels);
	if(isMember){
		if(targetChannel->getFlagTopic() == true){
			std::string error = "Topic restriction set to true, You need to be an operator.\n";
			client.sendMessage(error);
			bool isOperator = isOperatorFct(targetChannel,client);
			if(isOperator){
				if(targetChannel->getTopic() == topic){
					std::string message = "Topic already set to: " + topic + ".\n";
					client.sendMessage(message);
					return;
				}
				targetChannel->setTopic(topic);
				std::string message = "Channel topic set to: " + targetChannel->getTopic()+'\n';
				client.sendMessage(message);
				targetChannel->broadcastTopic(&client);
				std::cout << targetChannel->getName() + " has the topic set to: " + targetChannel->getTopic() <<std::endl;
			}
			else{
					std::string error = "You 're not an operator in " + channelName +  "\n";
					client.sendMessage(error);
			}
		}
		else{
			if(targetChannel->getTopic() == topic){
					std::string message = "Topic already set to: " + topic + ".\n";
					client.sendMessage(message);
					return;
				}
			targetChannel->setTopic(topic);
			std::string message = "Channel topic set to: " + targetChannel->getTopic()+'\n';
			client.sendMessage(message);
			targetChannel->broadcastTopic(&client);
			std::cout << targetChannel->getName() + " has the topic set to: " + targetChannel->getTopic() <<std::endl;
		}
	}
	else{
		std::string message = "You are not a member of this channel\n";
		client.sendMessage(message);
	}
	
}
		

void	Command::handleInvite(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string targetNick;
	stream >> targetNick;

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isOperator = isOperatorFct(targetChannel, client);
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			if(isOperator){
				Client *targetClient = _server.getClientByNick(targetNick);
				bool isMember = isMemberFct(targetChannel,*targetClient,channelName, channels);
				if(isMember){
					std::string response = targetNick + " already in the channel.\n";
					client.sendMessage(response);
					return ;
				}
				if (targetClient) {
					bool invitedAlready = false;
					std::vector<Client*> invited = targetChannel->getAllowedPeople();
					for (std::vector<Client*>::iterator It = invited.begin(); It != invited.end(); ++It) {
						if ((*It)->getNick() == targetNick) {
							invitedAlready = true;
							break;
						}
					}
					if(invitedAlready){
						std::string response = targetNick + " already invited in the " + channelName  + ".\n";
						client.sendMessage(response);
						return ;
					}
					targetChannel->addAllowedPeople(targetClient);
					std::string response = "You have been invited to join channel: " + channelName + "\n";
					targetClient->sendMessage(response);
				} else {
					std::string error = "User with nickname " + targetNick + " not found.\n";
					client.sendMessage(error);
				}
			} else {
				std::string error = "You are not an operator in :"  + targetChannel->getName() + " \n";
				client.sendMessage(error);
			}
			break;
		}
	}
	if (!found){
		std::string error = "You are not part of " + targetChannel->getName() +  " channel.\n";
		client.sendMessage(error);
	}
}

void	Command::handleMode(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std:: string flag;
	stream >> flag;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or no channel name given.\n";
		client.sendMessage(error);
		return;
	}
	if(flag.empty()){
		std::string error = "You need to insert one of the appropriate flags, i, t, k, o, l.\n";
		client.sendMessage(error);
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel: " + channelName + " does not exist.\n";
		client.sendMessage(error);
		return;
	}

	bool isOperator = isOperatorFct(targetChannel,client);
	if(isOperator){
		if(flag == "k"){
			std::string passWord;
			stream >> passWord;
			if(!modePass(passWord,client,channelName,targetChannel)) return ;
		}
		else if(flag == "i")
		{
			std::string mode;
			stream >> mode;
			if(!modeInvite(mode,targetChannel,client,channelName)) return ;
		}
		else if(flag == "o"){ 
			std::string name;
			stream>>name;
			if(!modeOperator(name,channels,client,channelName,_server)) return ;
		}
		else if(flag == "l"){
			std::string limit;
			stream >> limit;
			if(!modeLimit(limit,client, targetChannel,channelName)) return ;
		}
		else if(flag == "t"){
			std::string mode;
			stream >> mode;
			if(!modeTopic(mode,client,targetChannel,channelName)) return ;
		}
		else{
			std::string error = "You can only use one of the appropriate flags: i,t,k,o,l\n";
			client.sendMessage(error);
		}
	}
	else{
		std::string error = "You are not an operator in " + channelName + " channel.\n";
		client.sendMessage(error);
	}
}


