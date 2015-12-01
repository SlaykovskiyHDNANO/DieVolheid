#include "general_main.hpp"

const bool General::TRY_DEFAULT = true;
char* General::DEFAULT_ARGS[] = { "http_server", "0.0.0.0", "8080", "2", ".\n" };
const bool General::EXIT_CONFIRMATION = true;

int General::main_init_all(int argc, char** argv, shared_ptr<http::server::server> &serv, shared_ptr<utils::Logger> &log){

	log.reset(new utils::Logger(utils::LogOptions("server_log", ".txt", "0.5a", utils::LogType::LOG_INFO, false), true, true));
	mongo::client::initialize();

	// Check command line arguments.
	if (argc != 5)
	{
		std::cerr << "Usage: http_server <address> <port> <threads> <doc_root>\n";
		std::cerr << "  For IPv4, try:\n";
		std::cerr << "    http_server 0.0.0.0 80 1 .\n";
		std::cerr << "  For IPv6, try:\n";
		std::cerr << "    http_server 0::0 80 1 .\n";
		if (!General::TRY_DEFAULT)
			return 1;
		else {
			std::cerr << "However, as this binary compiled with TRY_DEFAULT = true, then defaults will be applied: ";
			for (int __i = 0; __i < 5; __i++)
				std::cerr << General::DEFAULT_ARGS[__i] << " ";
			std::cerr << ".\n";
			argv = General::DEFAULT_ARGS;
		}
	}

	//Connecting to DB
	//Deferred execution
	(*utils::Logger::Log) << "Running deferred connection to MongoDB" << std::endl;
	std::thread thread([]()->bool {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		bool ret = http::server::server::db_manager.Connect();
		(*utils::Logger::Log) << "Deferred execution ended. Result is: " << std::boolalpha << ret << std::endl;
		//MONGO MULTITHREADING DEBUG
		if (ret) {
			// make test connect
			auto acc = http::server::server::db_manager.GetAccessor();
			acc.StartAccess();
			//now, we will make 4 threads
			int num_threads = 4;
			std::vector<std::thread> th;
			for (int i = 0; i < num_threads; ++i){
				th.push_back(std::thread([](DBConnectorAccessor& acc){
					//connect
					auto kok = acc.GetById(SolvexDBObjectType::SOT_POST, SUUID("285ba61f-31e5-800f-0a45-3ea31ff61415"));

				},acc));
			}
			for (int i = 0; i < num_threads; ++i)
				th[i].join();

			acc.EndAccess();
			//
		}
		return ret;
	});

	thread.join();
	(*utils::Logger::Log) << "Running server loop..." << std::endl;
	// Initialise server.
	std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
	serv = shared_ptr<http::server::server>(new http::server::server(argv[1], argv[2], argv[4], num_threads));
	return 0;
}