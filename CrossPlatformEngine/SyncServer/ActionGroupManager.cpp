#include "ActionGroupManager.hpp"
#include "FindSimilar\FindSimilar.h"
#include "Preprocessor.hpp"
#include "server.hpp"
#include "utils\threading.hpp"


ActionReply::ActionReply(const string &SessionId, const string &UserId,
	const string &ActionId, const string &ActionNo, const string &result)
{
	this->SessionId = SessionId;
	this->UserId = UserId;
	this->ActionId = ActionId;
	this->ActionNo = ActionNo;
	this->result = result;
}

Json::Value ActionReply::GetJsonReply()
{
	Json::Value res;
	res["sessionid"] = SessionId;
	res["userid"] = UserId;
	res["action_id"] = ActionId;
	res["action_no"] = ActionNo;
	res["result"] = result;
	return res;
}

utils::threading::ThreadMessageQueue<Json::Value> ActionGroupManager::ResultQueue;	//и так сойдет...

ActionGroupManager::ActionGroupManager() :locker(new std::mutex()), active_handlers()
{
	this->SessionId = "";
	this->UserId = "";
	this->ActionId = "";
	this->initialized = false;
	this->msg_params = NULL;
	this->is_needed = true;
}

//TODO: WRAP CLASS TO AUTO_PTR
ActionGroupManager::ActionGroupManager(const ActionGroupManager& c) :locker(c.locker){
	this->SessionId = c.SessionId;
	this->active_handlers = c.active_handlers;
	this->UserId = c.UserId;
	this->ActionId = c.ActionId;
	this->initialized = c.initialized;
	this->msg_params = c.msg_params;
	this->is_needed = c.is_needed;
	this->ManagerThread = c.ManagerThread;
	this->MessageQueue = c.MessageQueue;
}


bool ActionGroupManager::ManagerFunction(ActionGroupManager* thiss)
{
	while (thiss->is_needed)
	{
		thiss->MessageQueue.wait();
		auto msg_params = thiss->MessageQueue.fpop();
		if (!thiss->initialized) {
			try {
				thiss->msg_params = msg_params;
				thiss->SessionId = msg_params["sessionid"].asString();
				thiss->UserId = msg_params["userid"].asString();
				thiss->ActionId = msg_params["action_id"].asString();
				thiss->initialized = true;
			}
			catch (exception e)
			{
				std::cerr << "FUCK: " << e.what();
				return false;
			}
		}
		if(msg_params["action_name"].asString() == "stop")
		{
			bool can_shutdown = false, failed = false;
			bool shutdown_all = true;
			for (auto it = thiss->active_handlers.begin(), end = thiss->active_handlers.end(); it != end; ++it)
			{
				can_shutdown = ((IActionHandler*)it->first)->CanGracefullyShutdown();

				if (can_shutdown) {
					failed = !((IActionHandler*)it->first)->Shutdown();
					//join thread
					//wait for WAIT_KILL_THREAD_TIMEOUT secs
					//if not killed, kill it forcefully OR WRITE TO LOG, THAT THREAD DOESN'T KILLED
					failed = true;

				}
				if (!can_shutdown || failed) {
					auto native = ((std::thread*)it->second)->native_handle();
					shutdown_all &= utils::threading::KillNativeThread(native);
					delete it->second;
				}

			}
			thiss->active_handlers.clear();
			//after all threads are shut down, kill this thread
			return shutdown_all;
		}
		else if (msg_params["action_name"].asString() == "post.preprocess" || msg_params["action_name"].asString() == "post.findsimilars"){
			ThreadActionHandlerParam* par = new ThreadActionHandlerParam();
			(*thiss->locker).lock();
			par->accessor = &http::server::server::db_manager.GetAccessor();
			(*thiss->locker).unlock();
			par->daddy = thiss;
			par->params = thiss->msg_params;
			par->ResultQueue = &thiss->ResultQueue;
			IActionHandler* action;
			if (msg_params["action_name"].asString() == "post.findsimilars")
				action = new FindSimilar();
			else
				action = new Preprocessor();
			
			auto thr = new thread([](IActionHandler* action, ThreadActionHandlerParam* par)->bool{
				action->Run(par->params, *par->accessor, *par->ResultQueue/*, true*/);	//true для бинарного поиска, false для линейного
				return true;
			}, action, par);
			thiss->active_handlers.insert(std::make_pair(action,thr));
		}
	}
	return true;
}

void ActionGroupManager::Start()
{
	this->ManagerThread = new std::thread(&ManagerFunction, this);
}

void ActionGroupManager::put(Json::Value val)
{
	this->MessageQueue.push(val);
}

string ActionGroupManager::get_sessionid()
{
	return this->SessionId;
}

string ActionGroupManager::get_userid()
{
	return this->UserId;
}

string ActionGroupManager::get_actionid()
{
	return this->ActionId;
}

Json::Value ActionGroupManager::get_ResultFromQueue()
{
	(*this->locker).lock();
	Json::Value res = ResultQueue.fpop();
	(*this->locker).unlock();
	return res;
}