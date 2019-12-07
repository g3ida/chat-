#pragma once

#include <string>
#include <optional>
#include <ctime>
#include "json/json.hpp"

namespace chatpp {
	
	struct message {
		enum class type {
			public_key,
			nickname,
			text
		};

		message() = default;
		message(const message&) = default;
		message(type t, std::string content, std::optional<unsigned int> tt = std::nullopt) : 
			message_type(t), content(content) {
			if (tt) {
				timestamps = tt.value();
			}
			else {
				std::time_t now = std::time(0);
				std::tm* now_tm = std::gmtime(&now);
				timestamps = static_cast<decltype(timestamps)>(std::mktime(now_tm));
			}
		}

		type message_type;
		std::string content;
		unsigned int timestamps;
	};

	void to_json(nlohmann::json& j, const message& m) {
		j = nlohmann::json{ 
			{"type", static_cast<int>(m.message_type)},
			{"content", m.content},
			{"timestamps", m.timestamps}
		};
	}

	void from_json(const nlohmann::json& j, message& m) {
		j.at("type").get_to(static_cast<message::type>(m.message_type));
		j.at("content").get_to(m.content);
		j.at("timestamps").get_to(m.timestamps);
	}
}