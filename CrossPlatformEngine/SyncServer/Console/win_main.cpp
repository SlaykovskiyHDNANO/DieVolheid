#include "general_main.hpp"

#if defined(_WIN32)
#include <Windows.h>

boost::function0<void> console_ctrl_function;

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		console_ctrl_function();
		return TRUE;
	default:
		return FALSE;
	}
}
#endif




int main(int argc, char* argv[])
{
	try
	{
		std::shared_ptr < utils::Logger> log;
		
		std::shared_ptr<http::server::server> s;
		General::main_init_all(argc, argv, s, log);

		
		// Set console control handler to allow server to be stopped.
		console_ctrl_function = boost::bind(&http::server::server::stop, s);
		SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

		// Run the server until stopped.
		s->run();
		cout << "Server ended its work." << std::endl;
		
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << std::endl;
	}

	if (General::EXIT_CONFIRMATION) {
		std::cout << "Please, press any key. Program will close...";
		std::cin.get();
	}


	return 0;
}

