#pragma once
#include <memory>
#include <vector>
#include <halley/text/halleystring.h>
#include "iconnection.h"
#include <boost/asio/detail/addressof.hpp>
#include "../../../src/udp_connection.h"

namespace Halley
{
	class NetworkServicePImpl;

	enum class IPVersion
	{
		IPv4,
		IPv6
	};

	class NetworkService
	{
	public:
		NetworkService(int port, IPVersion version = IPVersion::IPv4);
		~NetworkService();

		void update();

		void setAcceptingConnections(bool accepting);
		std::shared_ptr<IConnection> tryAcceptConnection();
		std::shared_ptr<IConnection> connect(String address, int port);

	private:
		std::unique_ptr<NetworkServicePImpl> pimpl;
		bool acceptingConnections = false;
		bool startedListening = false;

		void startListening();
		void receiveNext();
		void onNewConnectionRequest(char* data, size_t size, const UDPEndpoint& remoteEndpoint);
	};
}