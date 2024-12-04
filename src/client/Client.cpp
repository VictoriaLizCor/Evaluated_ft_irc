#include "Client.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int fd) : _clientFD(fd), _authenticated(false), nickname(""), username(""), _buffer("") {}

void Client::sendMessage(const std::string &message)
{
	write(_clientFD, message.c_str(), message.length());
}

Client::~Client() {}

void Client::handleRead() {
	char buffer[MAX_BUFFER];
	int nbytes = recv(_clientFD, buffer, sizeof(buffer) - 1, 0);
	if (nbytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // No data available
		else
			throw std::runtime_error("Error on recv: " + std::string(strerror(errno)));
	} else if (nbytes == 0) {
		throw std::runtime_error("Client disconnected");
	}
	buffer[nbytes] = '\0';
	this->_buffer += buffer;
	// std::cout << "Message: " << buffer << std::endl;
	// printAsciiDecimal(buffer);
	// std::cout << "Message_: " << _buffer << std::endl;
	// printAsciiDecimal(buffer);

	// Process commands
	Server* server = Server::getInstance();
	CommandParser commandParser(*server);
	size_t pos;
	while ((pos = this->_buffer.find_first_of("\r\n\0")) != std::string::npos)
	{
		std::string command = this->_buffer.substr(0, pos);
		this->_buffer.erase(0, pos + 1); // check if 2 or 1
		if (!this->_buffer.empty() && this->_buffer[0] == '\n') {
			this->_buffer.erase(0, 1);
		}
		// std::cout << "Received command from " << _clientFD << ": " << command << std::endl;
		// printAsciiDecimal(command);
		// std::cout << "Updated _buffer: " << this->_buffer << std::endl;
		// printAsciiDecimal(this->_buffer);
		// Handle command
		commandParser.parseAndExecute(*this, command, server->getChannels());
	}
}

bool Client::isAuthenticated() const {
	return _authenticated;
}

void Client::setAuthenticated(bool flag) {
	_authenticated = flag;
}

int Client::getFd() const
{
	return _clientFD;
}
