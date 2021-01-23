#ifndef __DEF_QUEUE_H___
#define __DEF_QUEUE_H___

#include "concurrentqueue.h"

#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <map>
#include <condition_variable>

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#endif

#include <websocketpp/server.hpp>

namespace chatpp {
	class message_queue {
	private:
		moodycamel::ConcurrentQueue<std::pair<std::string, std::string>> messages_queue;
		std::mutex _mut;
		std::condition_variable _data_cond;
	public:
		template <class T>
		void enqueue(T&& e) {
			messages_queue.enqueue(std::forward<T>(e));
			_data_cond.notify_one();
		}

		std::pair<std::string, std::string> wait_pop_message() {
			std::unique_lock<std::mutex> lk(_mut);
			std::pair<std::string, std::string> message;
			_data_cond.wait(lk, [this] { return messages_queue.is_lock_free() && messages_queue.size_approx() > 0; });
			bool res = messages_queue.try_dequeue(message);
			if (res) {
				return message;
			}
			//should not happen
			return wait_pop_message();
		}
	};
}

#endif