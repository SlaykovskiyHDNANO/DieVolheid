#pragma once
#ifndef IACTION_HANDLER_H
#define IACTION_HANDLER_H
#include <ctime>
#include <map>
#include <string>
#include <exception>
#include "DBConnector.hpp"
#include"JSON/json-forwards.h"
#include"JSON/json.h"
#include "utils\threading.hpp"

using namespace std;


class ActionResult
{
public:
	Json::Value result;
	bool is_done;
	exception e;
	bool has_errors;



	//Action
	
	Json::Value GetResult() { return result; }
	void Wait(int timeout)
	{
		//time_t beg = clock();
		time_t lim = clock() + timeout;
		while (clock()<lim && !is_done && !has_errors){}
	}
	bool IsDone(){ return is_done; }
	bool HasErrors(){ return has_errors; }
	std::exception GetException(){ return e; }
};


class IActionHandler 
{
public:
		
	virtual void Run(const Json::Value &data, DBConnectorAccessor &DB, utils::threading::ThreadMessageQueue<Json::Value>& queue) = 0;
	virtual bool Shutdown()=0;
	virtual ActionResult* GetResult()=0;
	virtual bool CanGracefullyShutdown()=0;
	
};
#endif