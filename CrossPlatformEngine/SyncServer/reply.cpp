#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <boost\bind.hpp>
#include <boost/asio.hpp>
#include "JSON\json\json.h"
#include "utils\Logger.h"
#include "server.hpp"
#include "ActionGroupManager.hpp"
#include "connection.hpp"
#include "reply.hpp"

namespace http {
	namespace server {
		using namespace std;


		void ReplySender::handle_reply(const boost::system::error_code& error, std::size_t bytes_transferred)
			{
				if (!error)
					cerr<< "ReplySender::handle_reply: everything is ok :D" << std::endl;
				else
					cerr << "ReplySender::handle_reply: PIZDEC :D" << error << std::endl;
			}

			bool ReplySender::SendReply()
			{
				bool message_received = false;
				do {
					message_received = ActionGroupManager::ResultQueue.wait(50);
					if (!this->_connected)
						this->needed = false;

					if (message_received && this->_connected){
						Json::Value reply = ActionGroupManager::ResultQueue.fpop();
						Json::FastWriter fw;
						auto str = fw.write(reply);
						unsigned char size[4];
						stringstream ss;
						size_t n = str.size();
						size[0] = (n >> 24) & 0xFF; size[1] = (n >> 16) & 0xFF;
						size[2] = (n >> 8) & 0xFF;  size[3] = n & 0xFF;
						//ss << size << str;
						//str = ss.str();
						//ss.clear();
						socket.send(boost::asio::buffer(size, 4));
						boost::asio::async_write(socket, boost::asio::buffer(str.c_str(), str.size()),
							strand.wrap(boost::bind(&ReplySender::handle_reply, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
					}
				} while (this->needed);
				return true;
			}

			ReplySender::ReplySender(const std::string &ip,const  std::string &port, boost::asio::io_service& service) : strand(service),
				socket(service), needed(true){
				// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
				boost::asio::ip::tcp::resolver resolver(service);
				boost::asio::ip::tcp::resolver::query query(ip, port);
				this->ep = *resolver.resolve(query);
			
			}

			void ReplySender::on_connect(const boost::system::error_code& error)
			{
				if (!error) {
					*(utils::Logger::Log) << "connected to front-end server" << std::endl;
					this->sender_thread = new std::thread(&ReplySender::SendReply, this);
					this->_connected = true;
				}
				else {
					*(utils::Logger::Log) << "Could not connect to front-end server: [Error=" << error <<  "]" << std::endl;
					this->_connected = false;
				}
			}

			bool ReplySender::connected(){
				return this->_connected;
			}

			void ReplySender::Start()
			{
				socket.open(boost::asio::ip::tcp::v4());
				//socket.connect(ep);
				//this->on_connect(boost::system::error_code());
				
				socket.async_connect(ep, strand.wrap(boost::bind(&ReplySender::on_connect, this->shared_from_this(), boost::asio::placeholders::error)));
			}

			ReplySender::~ReplySender()
			{
				this->needed = false;
				this->sender_thread->join();
			}

	} // namespace server
} // namespace http.

