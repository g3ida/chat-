#pragma once

#include <string>
#include <ctime>
#include "json/json.hpp"
#include "message.hpp"

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#endif

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using namespace std::string_literals;


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;


namespace chatpp {
	class chat_client {
	public :

		chat_client() {
			client_.set_access_channels(websocketpp::log::alevel::all);
			client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

			// Initialize ASIO
			client_.init_asio();

			// Register our message handler
			client_.set_message_handler(bind(&chat_client::on_message, this, &client_, ::_1, ::_2));

		}

		void start(std::string URI) {
			try {
				websocketpp::lib::error_code ec;
				cnx_ptr_ = client_.get_connection(URI, ec);
				if (ec) {
					std::cout << "could not create connection because: " << ec.message() << std::endl;
					throw "could not create connection";
				}

				// Note that connect here only requests a connection. No network messages are
				// exchanged until the event loop starts running in the next line.
				cnx_ptr_ = client_.connect(cnx_ptr_);
				
				std::thread t([this]() { client_.run(); });
				t.detach();
			}
			catch (websocketpp::exception const& e) {
				std::cout << e.what() << std::endl;
				throw;
			}
		}



		// This message handler will be invoked once for each incoming message. It
		// prints the message and then sends a copy of the message back to the server.
		void on_message(websocketpp::client<websocketpp::config::asio_client>* c, websocketpp::connection_hdl hdl, message_ptr msg) {
			
			//std::cout << "on_message called with hdl: " << hdl.lock().get()
			//	<< " and message: " << msg->get_payload()
			//	<< std::endl;


			websocketpp::lib::error_code ec;

			c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
			if (ec) {
				std::cout << "Echo failed because: " << ec.message() << std::endl;
			}
		}

		void send(std::string msg) {

			nlohmann::json msg_json_object;
			to_json(msg_json_object, message(message::type::text, msg));
			std::string json_str = msg_json_object.dump();

			client_.send(cnx_ptr_->get_handle(), json_str.data(), json_str.size()+1, websocketpp::frame::opcode::text);
		}

		void send_nickname(std::string nickname) {

			nlohmann::json msg_json_object;
			to_json(msg_json_object, message(message::type::nickname, nickname));
			std::string json_str = msg_json_object.dump();

			std::cout << json_str << std::endl;
			std::cout << json_str.size() << std::endl;
			if (cnx_ptr_) {
				std::cout << cnx_ptr_ << "valid]\n";
			}
			client_.send(cnx_ptr_->get_handle(), json_str.data(), json_str.size() + 1, websocketpp::frame::opcode::text);
		}

	private :
		websocketpp::client<websocketpp::config::asio_client>::connection_ptr cnx_ptr_;
		websocketpp::connection_hdl hdl_;
		websocketpp::client<websocketpp::config::asio_client> client_;
	};
}