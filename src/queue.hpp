#ifndef __DEF_QUEUE_H___
#define __DEF_QUEUE_H___

#ifdef _WIN32
	#include <winsock2.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "concurrentqueue.h"

#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>

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