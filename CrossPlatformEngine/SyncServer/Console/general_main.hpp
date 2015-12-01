#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <locale>
#include <codecvt>
#include <string>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <locale.h>



#include "Logger.h"
#include "server.hpp"
#include "DBConnector.hpp"
#include "utfconverter.h"

namespace General {
	//STATIC ARGUMENT CONSTANT
	//STATIC ARGUMENT CONSTANT
	extern const bool TRY_DEFAULT;
	extern char* DEFAULT_ARGS[];
	extern const bool EXIT_CONFIRMATION;

	extern int main_init_all(int argc, char** argv, shared_ptr<http::server::server> &serv, shared_ptr<utils::Logger> &log);
	
}





