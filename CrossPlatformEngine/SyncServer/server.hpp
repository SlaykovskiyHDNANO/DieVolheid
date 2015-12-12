#pragma once
#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "reply.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "ActionGroupManager.hpp"
//#include "DBConnector.hpp"
#include <memory>

namespace http {
	namespace server {

		/// The top-level class of the HTTP server.
		class server: private boost::noncopyable
		{
		public:
			/// Construct the server to listen on the specified TCP address and port, and
			/// serve up files from the given directory.
			server(const std::string& address, const std::string& port,
				const std::string& doc_root, std::size_t thread_pool_size);
			
			
			static std::vector<std::shared_ptr<ActionGroupManager>> ActionGroupList;
			/*static DBConnectorManager db_manager;*/
			static std::vector<std::shared_ptr<ReplySender>> ReplySenders;
			static const boost::asio::ip::tcp::endpoint frontend_endpoint;
			/// Run the server's io_service loop.
			void run();

			/// Stop the server.
			void stop();

			boost::asio::io_service& ios();
		private:
			/// Handle completion of an asynchronous accept operation.
			void handle_accept(const boost::system::error_code& e);

			/// The number of threads that will call io_service::run().
			std::size_t thread_pool_size_;

			/// The io_service used to perform asynchronous operations.
			//TODO: rename or make static accessor
			boost::asio::io_service io_service_;

			/// Acceptor used to listen for incoming connections.
			boost::asio::ip::tcp::acceptor acceptor_;

			/// The next connection to be accepted.
			connection_ptr new_connection_;
		};

	} // namespace server
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP