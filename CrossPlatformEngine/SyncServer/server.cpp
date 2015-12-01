#include "server.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace http {
	namespace server {

		DBConnectorManager server::db_manager("solvex", "backend", "backendbackend", 2, false);
		std::vector<std::shared_ptr<ActionGroupManager>> server::ActionGroupList		=	std::vector<std::shared_ptr<ActionGroupManager>>();
		std::vector<std::shared_ptr<ReplySender>>		 server::ReplySenders			=	std::vector<std::shared_ptr<ReplySender>>();

		const boost::asio::ip::tcp::endpoint	server::frontend_endpoint	=	boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 9998);
		server::server(const std::string& address, const std::string& port,
			const std::string& doc_root, std::size_t thread_pool_size)
			: thread_pool_size_(thread_pool_size),
			acceptor_(io_service_),
			new_connection_(new connection(io_service_))
		{
			
			// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
			boost::asio::ip::tcp::resolver resolver(io_service_);
			boost::asio::ip::tcp::resolver::query query(address, port);
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
			acceptor_.open(endpoint.protocol());
			acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			acceptor_.bind(endpoint);
			acceptor_.listen();
			acceptor_.async_accept(new_connection_->socket(),
				boost::bind(&server::handle_accept, this,
				boost::asio::placeholders::error));
		}

		void server::run()
		{
			// Create a pool of threads to run all of the io_services.
			std::vector<boost::shared_ptr<boost::thread> > threads;
			for (std::size_t i = 0; i < thread_pool_size_; ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &io_service_)));
				threads.push_back(thread);
			}

			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads.size(); ++i)
				threads[i]->join();
		}

		void server::stop()
		{
			io_service_.stop();
		}

		boost::asio::io_service& server::ios() {
			return this->io_service_;
		}

		void server::handle_accept(const boost::system::error_code& e)
		{
			if (!e)
			{
				this->new_connection_->start();
				//it would cause memory leak if it wan't shared ptr
				//it also would be dangerous, but rest assured: old shared_ptr will be deleted after all work on old this->new_connection_ is done
				this->new_connection_ = connection_ptr(new connection(io_service_));
				this->acceptor_.async_accept(new_connection_->socket(),
					boost::bind(&server::handle_accept, this,
					boost::asio::placeholders::error));
			}
			else
				std::cerr << e;
		}

	} // namespace server
} // namespace http