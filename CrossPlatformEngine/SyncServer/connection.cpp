#include "connection.hpp"
#include "server.hpp"
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include "byte_swaper.hpp"
#include <json\json.h>
#include "Logger.h"

namespace http {
	namespace server {
		using namespace std;

#pragma region Static Initializers
		std::vector<std::string> connection::valid_act_names = {"post.preprocess", "post.findsimilars"};
#pragma endregion

#pragma region Accessors
		boost::asio::ip::tcp::socket& connection::socket()
		{
			return socket_;
		}

		boost::asio::io_service::strand connection::get_strand()
		{
			return strand_;
		}
#pragma region Constructors
		connection::connection(boost::asio::io_service& io_service): strand_(io_service),
			socket_(io_service), no_replys_created(true)
		{

		}
#pragma endregion
#pragma region Control Functions
		void connection::start()
		{
			auto straddr = this->socket_.remote_endpoint();
			if (no_replys_created)
			{
				std::shared_ptr<ReplySender> p(new ReplySender("127.0.0.1","9998", this->socket_.get_io_service()));
				p->shared_from_this();
				server::ReplySenders.push_back(p);
				p->Start();
				no_replys_created = false;
				*(utils::Logger::Log)  << "WELCOME, NNNIGGA(" << straddr << ")!: He wants to tell you something..." << endl;
			}
			try {
				*(utils::Logger::Log)  << "WAITING FOR NIGGA("<< straddr <<") INPUT..." << std::endl;
				socket_.async_read_some(boost::asio::buffer(buffer_.data(),4),
					strand_.wrap(
					boost::bind(&connection::handle_first_bytes, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
				//boost::asio::read(socket_, boost::asio::buffer(buf, 4));
				//memcpy(buffer_.begin(), buf, 4);
				//handle_first_bytes(boost::system::error_code(), 4);

			}
			catch(...){
				*(utils::Logger::Log)  << "MAAFUCKER! This NIGGA("<<straddr<<") is dead, bro..." << endl;
				boost::system::error_code ignored_ec;
				socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
				socket_.close();
			}
				//strand_.wrap(
				//boost::bind(&connection::handle_first_bytes, shared_from_this(),
				//boost::asio::placeholders::error, 4)));
		}

		void connection::handle_first_bytes(const boost::system::error_code& e,
			std::size_t bytes_transferred)
		{
			if (e) {
				socket_.close();
				return;
			}
			//swap_byteorder((buffer_.data()), (size_t)4);
			int bytes = (int)buffer_.data();
			memcpy(&bytes, buffer_.data(), 4);

			//@WHERE_DELETED This will be deleted below
			char* data = new char[bytes]; 
			try {
				boost::asio::read(socket_, boost::asio::buffer(data, bytes));
			}
			catch (exception e){

			}
			//@WHERE_DELETED This will be deleted in connection::handle_write
			char* rep = new char[4];
			Json::Value value;
			Json::Reader reader;
			reader.parse(data, data + bytes, value, false);
			if (std::find(connection::valid_act_names.begin(), connection::valid_act_names.end(),
				value["action_name"].asString()) != connection::valid_act_names.end())
			{
				if (!(!value["sessionid"] || !value["userid"]
					|| !value["action_no"] || !value["action_id"]
					|| !value["action_name"]))
				{
					*(utils::Logger::Log)  << "OK, NIGGA!" << endl;
					memcpy(rep, "OKAY", 4);
					// ÄÀËÜØÅ ÂÑÅ ÄÅÐÜÌÎ, ÍÀÄÎ ÌÅÍßÒÜ
					bool is_found = false;
					for (int i = 0; i < http::server::server::ActionGroupList.size(); i++)
						if (value["action_id"] == http::server::server::ActionGroupList[i]->get_actionid() &&
							value["user_id"] == http::server::server::ActionGroupList[i]->get_userid() &&
							value["sessionid"] == http::server::server::ActionGroupList[i]->get_sessionid()){
							http::server::server::ActionGroupList[i]->put(value);
							is_found = true;
						}
					if (!is_found)
					{
						//@WHERE_DELETED This will be deleted by shared_ptr
						shared_ptr<ActionGroupManager> new_manager(new ActionGroupManager);
						http::server::server::ActionGroupList.push_back(new_manager);
						new_manager->put(value);
						new_manager->Start();
					}
					else
					{
						*(utils::Logger::Log)  << "WTF YOU JUST SAY, NIGGA?" << endl;
						memcpy(rep, "FAIL", 4);
					}
				}
			}
			else
			{
				*(utils::Logger::Log) << "WTF YOU JUST SAY, NIGGA?" << endl;
				memcpy(rep, "FAIL", 4);
			}
			//@WHERE_DELETED THIS IS ALLOWED
			delete data;
			
			boost::asio::async_write(this->socket_, boost::asio::buffer(rep, 4), boost::bind(&connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
//			handle_write(boost::system::error_code(),rep, 4);
				//@WHERE_DELETED THIS IS ALLOWED
			delete rep;
		}
		

		void connection::handle_write(const boost::system::error_code& e, size_t rep)
		{
			if (!e)
			{
				//swap_byteorder(rep, size);
				//delete rep;
				start();
			}
		}
#pragma endregion

	} // namespace server
} // namespace http