#ifndef __DEF_SERVER_H___
#define __DEF_SERVER_H___

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#endif

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include "queue.hpp"
#include "message.hpp"

#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <map>
#include <thread>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr client_message_ptr_t;

namespace chatpp {
	class chat_server {
	public :
		chat_server(websocketpp::log::level log_level = websocketpp::log::alevel::none) {
			// Set logging settings
			server_.set_access_channels(log_level);
			server_.clear_access_channels(websocketpp::log::alevel::frame_payload);

			// Initialize Asio
			server_.init_asio();
			
			// Register our message handler
			server_.set_message_handler(bind(&chat_server::on_recieve_message, this, &server_, ::_1, ::_2));
		}

		// Callback to handle incoming messages
		void on_recieve_message(server*, websocketpp::connection_hdl hdl, client_message_ptr_t msg) {

			if (msg->get_opcode() == websocketpp::frame::opcode::text) {

				try {
					std::stringstream ss;
					ss << msg->get_payload();
				
					message m;
					nlohmann::json j;
		
					j << ss;
					j.get_to(m);
					switch (m.message_type) {
					case message::type::nickname:
						if (auto it = get_nickname(hdl); it != nicknames_.end()) {
							 (*it) = m.content;
						}
						else {
							handlers_.emplace_back(hdl);
							nicknames_.emplace_back(m.content);
						}
						break;
					case message::type::public_key:
						break;
					case message::type::text:
						msg_queue_.enqueue(std::make_pair(*get_nickname(hdl), std::move(m.content)));
						break;
					}
				}
				catch (std::exception& e) {
					std::cout << e.what() << std::endl;
				}
			}
		}

		void start(int port = 9002) {
			server_.listen(port);
			// Start the server accept loop
			server_.start_accept();
			// Start the ASIO io_service run loop in another thread
			std::thread t([this]() { server_.run(); });
			t.detach();
		}

		std::vector<std::string>::iterator get_nickname(websocketpp::connection_hdl hdl) {
			if (!hdl.expired()) {
				auto cnx = hdl.lock();
				for (int i = 0; i < handlers_.size(); ++i) {
					if (!handlers_[i].expired()) {
						if (handlers_[i].lock() == cnx) {
							return (nicknames_.begin() + i);
						}
					}
				}
			}
			return nicknames_.end();
		}

		message_queue msg_queue_;
	private :
		std::vector<websocketpp::connection_hdl> handlers_;
		std::vector<std::string> nicknames_;
		websocketpp::server<websocketpp::config::asio> server_;
	};
}

#endif