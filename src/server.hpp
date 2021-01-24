#ifndef __DEF_SERVER_H___
#define __DEF_SERVER_H___

#ifdef _WIN32
	#include <winsock2.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_THREAD_
#endif

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "client.hpp"
#include "queue.hpp"

class vector;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace chatpp {

	using server = websocketpp::server<websocketpp::config::asio>;

	class chat_server {
	public :
		chat_server(websocketpp::log::level log_level = websocketpp::log::alevel::none);
		void on_recieve_message(server*, websocketpp::connection_hdl hdl, client_message_ptr_t msg);
		void start(int port = 9002);
		std::vector<std::string>::iterator get_nickname(websocketpp::connection_hdl hdl);

		message_queue msg_queue_;
	private :
		std::vector<websocketpp::connection_hdl> handlers_;
		std::vector<std::string> nicknames_;
		websocketpp::server<websocketpp::config::asio> server_;
	};
}

#endif