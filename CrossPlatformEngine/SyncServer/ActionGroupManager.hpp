#pragma once
#include <string>
#include <vector>
#include <thread>
//#include "DBConnector.hpp"
#include "IActionHandler.h"
#include "utils\threading.hpp"
#include "JSON\json\json.h"

using namespace std;


class ActionReply
{
private:
	string SessionId;
	string UserId;
	string ActionId;
	string ActionNo;
	string result;
public:
	ActionReply(const string &SessionId, const string &UserId,
		const string &ActionId, const string &ActionNo, const string &result);
	Json::Value GetJsonReply();
};

class ActionGroupManager;

struct ThreadActionHandlerParam {
	ActionGroupManager* daddy;
	/*DBConnectorAccessor* accessor;*/
	Json::Value params;
	utils::threading::ThreadMessageQueue<Json::Value>* ResultQueue;

};

class ActionGroupManager
{
private:
	utils::threading::ThreadMessageQueue<Json::Value> MessageQueue;   
	mutable std::shared_ptr<std::mutex> locker;
	string	SessionId;
	string	UserId;
	string	ActionId;
	bool initialized;
	Json::Value msg_params;
	thread* ManagerThread;
	std::map<IActionHandler*, std::thread*> active_handlers;
	bool    is_needed;
	//функция обработки связанной группы действий
	static bool ManagerFunction(ActionGroupManager* thiss);
public:
	static utils::threading::ThreadMessageQueue<Json::Value> ResultQueue;	//и так сойдет...
	ActionGroupManager();
	ActionGroupManager(const ActionGroupManager&);
	string get_sessionid();
	string get_userid();
	string get_actionid();
	void Start();
	void Stop();
	void put(Json::Value val);
	Json::Value get_ResultFromQueue();
};




