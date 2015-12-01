#pragma once
#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"


namespace http {
	namespace server {

		/// Represents a single connection from a client.
		class connection
			: public boost::enable_shared_from_this<connection>,
			private boost::noncopyable
		{
		public:

			static std::vector<std::string> valid_act_names;

			/// Construct a connection with the given io_service.
			explicit connection(boost::asio::io_service& io_service);
			/// Get the socket associated with the connection.
			boost::asio::ip::tcp::socket& socket();

			/// Start the first asynchronous operation for the connection.
			void start();

			boost::asio::io_service::strand get_strand();
		private:
			/// Handle completion of a write operation.
			void handle_write(const boost::system::error_code& e, size_t size);

			// Handle to read first bytes
			void handle_first_bytes(const boost::system::error_code& e,
				std::size_t bytes_transferred);

			/// Strand to ensure the connection's handlers are not called concurrently.
			boost::asio::io_service::strand strand_;

			/// Socket for the connection.
			boost::asio::ip::tcp::socket socket_;

			/// Buffer for incoming data.
			boost::array<char, 8192> buffer_;

			bool no_replys_created;
			/// The reply to be sent back to the client.
			//reply reply_;

		};

		typedef boost::shared_ptr<connection> connection_ptr;

	} // namespace server
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP