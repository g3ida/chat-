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

namespace chatpp {
	class chat_client {
	public :
		using client_t = websocketpp::client<websocketpp::config::asio_client>;
		using client_connection_ptr_t = websocketpp::client<websocketpp::config::asio_client>::connection_ptr;
		using client_message_ptr_t = websocketpp::config::asio_client::message_type::ptr;

		enum class status {
			open,
			closed,
			failed,
			connecting
		};

		chat_client(websocketpp::log::level log_level = websocketpp::log::alevel::none) {
			client_.set_access_channels(log_level);
			client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
			// Initialize ASIO
			client_.init_asio();
			// Register our message handler
			client_.set_message_handler(bind(&chat_client::on_message, this, &client_, ::_1, ::_2));
			client_.set_open_handler(websocketpp::lib::bind(&chat_client::on_open, this, &client_, ::_1));
			client_.set_close_handler(websocketpp::lib::bind(&chat_client::on_close, this, &client_, ::_1));
			client_.set_fail_handler(websocketpp::lib::bind(&chat_client::on_fail, this, &client_, ::_1));
		}

		void start(std::string URI) {
			try {
				websocketpp::lib::error_code ec;
				cnx_ptr_ = client_.get_connection(URI, ec);
				if (ec) {
					status_ = status::failed;
					status_msg_ = ec.message();
					throw new websocketpp::exception(ec);
				}
				// Note that connect here only requests a connection. No network messages are
				// exchanged until the event loop starts running in the next line.
				cnx_ptr_ = client_.connect(cnx_ptr_);
				status_ = status::connecting;
				std::thread t([this]() { client_.run(); });
				t.detach();
			}
			catch (websocketpp::exception const& e) {
				status_ = status::failed;
				status_msg_ = e.m_code.message();
				throw;
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
			client_.send(cnx_ptr_->get_handle(), json_str.data(), json_str.size() + 1, websocketpp::frame::opcode::text);
		}

		status getStatus() {
			return status_;
		}

		std::string getServerStr() {
			return server_str_;
		}

		std::string getErrorMessage() {
			return status_msg_;
		}

	private :

		void on_open(client_t* client, websocketpp::connection_hdl hdl) {
			status_ = status::open;
			client_t::connection_ptr con = client->get_con_from_hdl(hdl);
			server_str_ = con->get_response_header("Server");
		}

		void on_close(client_t* client, websocketpp::connection_hdl hdl) {
			status_ = status::closed;
			client_t::connection_ptr con = client->get_con_from_hdl(hdl);
			std::stringstream s;
			s << "close code: " << con->get_remote_close_code() << " ("
				<< websocketpp::close::status::get_string(con->get_remote_close_code())
				<< "), close reason: " << con->get_remote_close_reason();
			server_str_ = s.str();
		}

		void on_fail(client_t* client, websocketpp::connection_hdl hdl) {
			status_ = status::failed;
			client_t::connection_ptr con = client->get_con_from_hdl(hdl);
			server_str_ = con->get_response_header("Server");
			status_msg_ = con->get_ec().message();
		}

		void on_message(websocketpp::client<websocketpp::config::asio_client>* c, websocketpp::connection_hdl hdl, client_message_ptr_t msg) {
			websocketpp::lib::error_code ec;
			c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
			if (ec) {
				status_msg_ = ec.message();
			}
		}

		status status_ = status::closed;
		client_connection_ptr_t cnx_ptr_;
		websocketpp::connection_hdl hdl_;
		client_t client_;
		std::string server_str_;
		std::string status_msg_;
	};
}