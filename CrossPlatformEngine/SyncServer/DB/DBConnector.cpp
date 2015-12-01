#pragma once
#include "DBConnector.hpp"

#pragma region IDBConnectorResult

std::string MongoQueryResult::Raw() const {
	throw std::logic_error("Not implemented");
}

mongo::BSONObj MongoQueryResult::NextResult(bool own) const {
	return own ? this->_q->next().getOwned() : this->_q->next();
}

bool MongoQueryResult::HasError() const {
	return this->_q->peekError();
}

bool MongoQueryResult::HasMore() const {
	return this->_q->more();
}



#pragma endregion


#pragma region DBConnectorBase Implementation

DBConnectorStatus::DBConnectorStatus(){
	this->error = false;
	data = 0;
	connected = false;;
	initialized = false;;
	string errmsg = "";
	std::shared_ptr<mongo::DBException> excep = nullptr;
}

DBConnectorBase::DBConnectorBase():_m(new std::mutex()){
}

DBConnectorBase::DBConnectorBase(const DBConnectorBase& c):credentials(c.credentials), cn(c.cn),_m(c._m){

}

DBConnectorBase::DBConnectorBase(const string &dbname, const string &dbuser, const string &pwd, bool connect_now) : credentials(new DBConnectorCredentials()), _m(new std::mutex())
{
		this->credentials->dbname = dbname;
		this->credentials->dbuser = dbuser;
		this->credentials->dbpass = pwd;
		status.initialized = true;
		status.connected = false;
		status.data = NULL;
		status.errmsg = "";
		status.error = false;
	    this->cn = std::shared_ptr<mongo::DBClientConnection>(new mongo::DBClientConnection);
		if (connect_now)
			this->Connect();
}

DBConnectorBase::~DBConnectorBase()
{
	//delete this;
}

bool DBConnectorBase::Connect()
{
	
		try
		{
			(*cn).connect("ds034878.mongolab.com:34878");
			this->status.connected = (*cn).auth(credentials->dbname, credentials->dbuser, credentials->dbpass, status.errmsg);
		}
		catch (mongo::DBException e)
		{
			status.error = true;
			this->status.connected = false;
			status.excep.reset(new mongo::DBException(e));
			this->status.errmsg = e.what();
			return false;
		}
		return true;
}

bool DBConnectorBase::Reconnect()
{
	if (!status.error)
	{
		this->Disconnect();
		return this->Connect();
	}
	else
		return false;
}

bool DBConnectorBase::Disconnect()
{
	if (status.connected)
	{
		mongo::BSONObj info;    //сюда хорошо бы что-то передавать, но вроде как это необязательно
		(*cn).logout(credentials->dbname, info);
		status.connected = false;
	}
	return true;
}


void DBConnectorBase::SetCredentials(const string &dbname, const string &dbuser, const string &pwd, const bool &reconnect_now)
{
	if (dbname != "")
		this->credentials->dbname = dbname;
	if (dbuser != "")
		this->credentials->dbuser = dbuser;
	if (pwd != "")
		this->credentials->dbpass = pwd;
}

DBConnectorStatus DBConnectorBase::Status() const
{
	return this->status;
}

bool DBConnectorBase::IsConnected() const
{
	return this->status.connected;
}

string DBConnectorBase::LastErrorMsg() const
{
	return this->status.errmsg;
}

mongo::DBException DBConnectorBase::LastError() const
{
	return this->status.excep.operator*();
}



#pragma endregion

#pragma region DBConnectorWriter Implementation
DBConnectorWriter::DBConnectorWriter(const string &dbname, const string &dbuser,
	const string &pwd, const bool &connect_now) :DBConnectorBase(dbname,dbuser,pwd,connect_now)
{

}

DBConnectorWriter::~DBConnectorWriter()
{
}

bool DBConnectorWriter::Write(SolvexDBObjectType type, const mongo::BSONObj obj)
{
	string path;
	switch (type)
	{
	case SOT_TAG:
		path = credentials->dbname + ".tag";
		break;
	case SOT_SECTION:
		path = credentials->dbname + ".section";
		break;
	case SOT_PREPROCESSED_TEXT:
		path = credentials->dbname + ".preprocessedtext";
		break;
	case SOT_USER:
		path = credentials->dbname + ".user";
		break;
	case SOT_POST:
		path = credentials->dbname + ".post";
		break;
	}
	try
	{
		std::lock_guard<std::mutex> ll(*this->_m);
		(*cn).insert(path, obj);
	}
	catch (mongo::DBException e)
	{
		std::cerr << "DBConnectorWriter::There was an error when saving: "<< std::endl <<e.what();
		return false;
	}
	return true;
}

#pragma endregion

#pragma region DBConnectorReader implementation
 
int DBConnectorReader::seed = 0;

DBConnectorReader::DBConnectorReader(const string &dbname, const string &dbuser,
	const string &pwd, const bool &connect_now) :DBConnectorBase(dbname, dbuser, pwd, connect_now), sowed_seed(DBConnectorReader::seed++)
{
	
}

DBConnectorReader::DBConnectorReader():DBConnectorBase(),sowed_seed(DBConnectorReader::seed++){

}

DBConnectorReader::~DBConnectorReader()
{
}



mongo::BSONObj DBConnectorReader::GetById(SolvexDBObjectType type, const SUUID &id,
	const std::vector<string> &fields, bool* ok)
{
	string path;
	switch (type)
	{
	case SOT_TAG:
		path = credentials->dbname + ".tag";
		break;
	case SOT_SECTION:
		path = credentials->dbname + ".section";
		break;
	case SOT_PREPROCESSED_TEXT:
		path = credentials->dbname + ".preprocessedtext";
		break;
	case SOT_USER:
		path = credentials->dbname + ".user";
		break;
	case SOT_POST:
		path = credentials->dbname + ".post";
	}

	auto_ptr<mongo::DBClientCursor> ret;
	if (cn&&cn->isStillConnected() && !cn->isFailed()){
		//std::lock_guard<std::mutex> lm(this->_m); //lock up
		std::lock_guard<std::mutex> ll(*this->_m);
		
		if (id.IsInt())
			ret = (*cn).query(path, BSON("_id" << id.AsInt()));
		else
			ret = (*cn).query(path, BSON("_id" << id.AsStr()));
		
		if (ret->more())
			return ret->next().getOwned();		
		else
			throw exception("No results");
	}
	else
		throw exception("Exception Occured during sending query request");
}




//mongo::BSONObjSet DBConnectorReader::GetByQuery(SolvexDBObjectType type, const string &query_string,
//	const std::vector<string> &fields = {}, bool* ok = nullptr)
//{
//
//}



#pragma endregion

#pragma region DBConnectorAccessor Implementation

DBConnectorAccessor::DBConnectorAccessor():thread_id(0), access_granted(false), end_access_called(false), daddy(0), DBConnectorReader()
{

	this->valid = false;
}

DBConnectorAccessor::DBConnectorAccessor(const DBConnectorReader& reader, DBConnectorManager* daddy) :DBConnectorReader(reader), daddy(daddy)
{
	this->access_granted = false;
	this->end_access_called = false;
	this->thread_id = NULL;
	//need to be set in the end
	this->valid = true;
};

DBConnectorAccessor::DBConnectorAccessor(const DBConnectorAccessor& a) :
enable_shared_from_this<DBConnectorAccessor>(a), DBConnectorReader(a), daddy(a.daddy)
{
	//need to be set in the end
	this->valid = true;
};
//
//DBConnectorAccessor& DBConnectorAccessor::operator = (const DBConnectorAccessor& a){
//
//	((DBConnectorReader*)this)->operator=(((DBConnectorReader)a));
//	this->daddy = a.daddy;
//	this->access_granted = false;
//	this->thread_id = 0;
//	return *this;
//}

bool DBConnectorAccessor::StartAccess()
{
	if (!end_access_called) {
		this->access_granted = true;
		this->thread_id = std::this_thread::get_id().hash();
	}
	return this->access_granted;
}

bool DBConnectorAccessor::EndAccess()
{
	if (access_granted)
	{
		access_granted = false;
		end_access_called = true;
		daddy->ChildIsDone(*(this));
		return true;
	}
	else
		return false;
}

mongo::BSONObj DBConnectorAccessor::GetById(SolvexDBObjectType type, const SUUID &id, const std::vector<string> &fields, bool* ok)
{
	if (access_granted) {
		return DBConnectorReader::GetById(type, id, fields, ok);
	}
	throw new std::runtime_error("Access Denied");
}

bool DBConnectorAccessor::EnqueueWrite(SolvexDBObjectType type, const mongo::BSONObj &object) //тут не совсем понятно зачем использовать Rvalue ссылку и в целом назначение функции
{
	DBWriteEntry new_entry;
	new_entry.type = type;
	new_entry.obj = object;
	this->write_entries.push_back(new_entry);
	return true;
}

#pragma endregion

#pragma region DBConnectorManager Implementation

DBConnectorManager::DBConnectorManager(const string &dbname, const string &dbuser, const string &pwd, int number_of_connections, const bool &connect_now) :writer(dbname,dbuser,pwd,connect_now)
{
	for (int i = 0; i < number_of_connections; ++i){
		auto con = new DBConnectorReader(dbname, dbuser, pwd, connect_now);
		this->load.insert(std::pair<DBConnectorReader, std::vector<DBConnectorAccessor*>>(*con, *new std::vector<DBConnectorAccessor*>()));
	}
		
}

bool DBConnectorManager::IsConnected(bool* writer,bool* any_reader) {
	bool b = this->writer.IsConnected();
	if (writer != nullptr)
		*writer = b;
	bool any = false, all = b;

	for (auto it = this->load.begin(), end = this->load.end(); it != end; ++it) {
		register bool bw = it->first.IsConnected();
		any |= bw;
		all &= bw;
	}
	if (any_reader != nullptr)
		*any_reader = all;

	return all;
}

bool DBConnectorManager::Connect(bool recon){
	bool res = true;
	if (recon || !this->writer.IsConnected())
		res &= this->writer.Reconnect();

	for (std::map<DBConnectorReader, std::vector<DBConnectorAccessor*>>::iterator it = this->load.begin(), end = this->load.end(); it != end; ++it)
		if (recon || !it->first.IsConnected())
			res &= (const_cast<DBConnectorReader&>(it->first)).Reconnect();

	return res;
}

DBConnectorAccessor& DBConnectorManager::GetAccessor()
{
	this->locker.lock();
	auto it = load.begin();
	auto end = load.end();
	std::map<DBConnectorReader, std::vector<DBConnectorAccessor*>>::iterator free = it;
	if (it == end)
		throw new std::logic_error("WTF? No Readers to find");
	//IF OKAY
	int free_reader_size = it->second.size();
	for (it; it != end; it++)
	{
		if (it->second.size() < free_reader_size)
		{
			free_reader_size = it->second.size();
			free = it;
		}
	}
	DBConnectorAccessor* new_accessor = new DBConnectorAccessor(free->first, this);
	free->second.push_back(new_accessor);
	this->locker.unlock();
	return *(new_accessor);
}

bool DBConnectorManager::RemoveAccessorsByThreadId(int thread_id)
{
	this->locker.lock();
	std::map<DBConnectorReader, std::vector<DBConnectorAccessor*>>::iterator it;
	for (it = load.begin(); it != load.end(); it++)
	{
		for (int s=it->second.size(),i = s-1; i >=0; --i)
		{
			if ((*(it->second[i])).thread_id == thread_id){
				delete it->second[i];
				it->second.erase(it->second.begin() + i);
			}
		}
	}
	this->locker.unlock();
	return true;
}

bool DBConnectorManager::ChildIsDone(const DBConnectorAccessor& child)
{
	for (int i = 0; i < child.write_entries.size(); i++)
		writer.Write(child.write_entries[i].type, child.write_entries[i].obj);
	return this->RemoveAccessorsByThreadId(child.thread_id);
}
#pragma endregion