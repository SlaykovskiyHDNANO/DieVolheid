#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <JSON/json/json.h>

		namespace http {
			namespace server {
				using namespace std;

				class ReplySender : public std::enable_shared_from_this<ReplySender>
				{
				private:

					bool _connected;
					boost::asio::ip::tcp::socket socket;
					boost::asio::io_service::strand strand;
					boost::asio::ip::tcp::endpoint ep;
					std::thread* sender_thread;
					std::mutex locker;
					bool needed;

					void handle_reply(const boost::system::error_code& error, std::size_t bytes_transferred);
					void on_connect(const boost::system::error_code& error);
					bool SendReply();

				public:
					ReplySender(const std::string& ip, const std::string& port, boost::asio::io_service& service);
					bool connected();

					void Start();
					~ReplySender();
				};

		} // namespace server
} // namespace http.
			
