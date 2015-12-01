#pragma once
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <mongo/client/dbclient.h> // for the driver
#include <string>
#include <thread>
#include <mutex>
//SUUID - 4,8,12,16 байтовое число
//Все документы в нашей БД подписаны 16 байтным числом (строкой), кроме Секции и Языка
//эти подписаны (пока что) 32 байтным числом.
#include "suuid.hpp"

using namespace std;



enum SolvexDBObjectType {
	SOT_TAG,
	SOT_POST,
	SOT_USER,
	SOT_SECTION,
	SOT_PREPROCESSED_TEXT //хранится отдельно
};

class IDBConnectorResult {
public:
	virtual mongo::BSONObj	NextResult(bool own)	const	= 0;
	virtual bool			HasMore()				const	= 0;
	virtual bool			HasError()				const	= 0;
	virtual std::string		Raw()					const	= 0;
};

class MongoQueryResult :IDBConnectorResult {
protected:
	auto_ptr<mongo::DBClientCursor> _q;
public:
	mongo::BSONObj	NextResult(bool own)	const	override;
	bool			HasMore()				const	override;
	bool			HasError()				const	override;
	std::string		Raw()					const	override;
};


struct DBConnectorStatus {
	void* data;
	bool connected;
	bool initialized;
	bool error;
	string errmsg;
	std::shared_ptr<mongo::DBException> excep;

	DBConnectorStatus();
};
struct DBConnectorCredentials {
	string dbuser;
	string dbname;
	string dbpass;
};

class DBConnectorBase:public enable_shared_from_this<DBConnectorBase>
{
protected:
	DBConnectorStatus status;
	std::shared_ptr<mongo::DBClientConnection> cn;
	std::shared_ptr<DBConnectorCredentials> credentials;
	shared_ptr<std::mutex> _m;
public:
#pragma region Constructors and Destructors
	//Подключается к базе данных, через него проходят все запросы
	DBConnectorBase();
	DBConnectorBase(const DBConnectorBase&);
	DBConnectorBase(const string &dbname, const string &dbuser, const string &pwd, bool connect_now = true);
	~DBConnectorBase();
#pragma endregion
#pragma region Main
	bool Connect();
	bool Disconnect();
	bool Reconnect();
	//пустые значения dbname dbuser pwd не меняются
	void SetCredentials(const string &dbname = "",const string &dbuser = "",const string &pwd = "",const bool &reconnect_now = false);
#pragma endregion
#pragma region Misc
	DBConnectorStatus Status() const;
	bool IsConnected() const;
	string LastErrorMsg() const;
	mongo::DBException LastError() const;
#pragma endregion


};

class DBConnectorWriter :public DBConnectorBase
{
public:
#pragma region Constructors and Destructors
	//Подключается к базе данных, через него проходят все запросы
	DBConnectorWriter(){};
	DBConnectorWriter(const string &dbname, const string &dbuser, const string &pwd, const bool &connect_now = true);
	~DBConnectorWriter();
#pragma endregion

#pragma region Write-oriented methods
	bool Write(SolvexDBObjectType type, const mongo::BSONObj);
#pragma endregion

};

class DBConnectorReader:public DBConnectorBase
{
	static int seed;

	int sowed_seed;
public:
#pragma region Constructors and Destructors
	DBConnectorReader();
	DBConnectorReader(const string &dbname ,const string &dbuser, const string &pwd, const bool &connect_now = true);
	~DBConnectorReader();
#pragma endregion

#pragma region Read-oriented methods
	mongo::BSONObj GetById(SolvexDBObjectType type,const SUUID &id,const std::vector<string> &fields = {}, bool* ok = nullptr);
	//mongo::BSONObjSet GetByQuery(SolvexDBObjectType type,const string &query_string,const std::vector<string> &fields = {}, bool* ok = nullptr);
#pragma endregion

	friend bool DBConnectorReader::operator<(const DBConnectorReader& a, const DBConnectorReader& b) {
		return a.sowed_seed < b.sowed_seed;
	}

};

struct DBWriteEntry
{
	SolvexDBObjectType type;
	mongo::BSONObj obj;
};


class DBConnectorManager;

/** 
* Предоставляет доступ к базе данных всем обработчикам. Доступно также внесение редакций и создание новых документов.
* Каждый созданный 
* Этот класс ты будешь выдавать всем обработчикам
* Он тесно связан с DBConnectorManager, который упраляет нагрузкой на соединения
* Перед началом работы все классы должны выполнить функцию StartAccess()
* Все классы после отработки должны выполнить функцию EndAccess(). Все, что было подано на запись, запишется в mongoDB
**/
class DBConnectorAccessor:protected DBConnectorReader,public enable_shared_from_this<DBConnectorAccessor>
{
	DBConnectorManager* daddy;
	std::deque<DBWriteEntry> write_entries;
	int thread_id;
	bool access_granted;
	bool valid;
	bool end_access_called;
public:
	DBConnectorAccessor(const DBConnectorReader& reader, DBConnectorManager* daddy);
	DBConnectorAccessor(const DBConnectorAccessor&);
	DBConnectorAccessor();
	//Разрешает пользоваться базой данных. Запоминает id потока
	bool StartAccess();
	//После этого запрещает пользоваться базой данных. Вызывает DBConnectorManager::ChildIsDone() 
	bool EndAccess();
	//DBConnectorAccessor& operator=(const DBConnectorAccessor& a);
	//Здесь нужно проверять перед выполнением кода, выполнен ли вызов StartAccess()
	mongo::BSONObj GetById(SolvexDBObjectType type, const SUUID &id, const std::vector<string> &fields = {}, bool* ok = nullptr);
	//mongo::BSONObjSet GetByQuery(SolvexDBObjectType type, const string &query_string, const std::vector<string> &fields = {}, bool* ok = nullptr);
	
	bool EnqueueWrite(SolvexDBObjectType type, const mongo::BSONObj &object);
	//bool EnqueueWrite(SolvexDBObjectType type, const mongo::BSONObjSet &&objects);

	//Это означает, что классу DBConnectorManager доступны все поля из этого класса. Даже приватные
	friend class DBConnectorManager;
};

class DBConnectorManager
{
	//Так как эту херню вызывают много потоков, то мы будем разрешать доступ к вызовам только одному потоку,
	//остальные будут ждать окончания вызова от того потока.
	std::mutex locker;
	bool	connected;
	//словарь, в котором хранится – рид-соединение с монго и нагрузка (количество теущих DBConnectorAccessor на это соединение)
		
	DBConnectorWriter writer;
protected:
	//Эту функцию будут вызывать все DBConnectorAccessor после вызова EndAccess()
	//DBConnectorManager подает на запись все данные, которые отработавший обработчик подал на запись.
	bool ChildIsDone(const DBConnectorAccessor& child);
public:
	std::map<DBConnectorReader, std::vector<DBConnectorAccessor*>> load;
	DBConnectorManager(const string &dbname, const string &dbuser, const string &pwd, int number_of_connections = 2, const bool& connect_now = false);

	//connect
	bool Connect(bool reconnect_connected = false);

	//Checks, whether connection to DB is made
	//@arg		writer Place to store a status of writer connection
	//@arg		any_reader Place to store condition result, whether any of reader is connected
	//@return	bool value, indicating whether ALL connections (readers and writers are connected)
	bool IsConnected(bool* writer = nullptr, bool* any_reader = nullptr);

	//Генерирует новый DBAccessor.
	//Выбирает DBConnectorReader с наименьшим количеством DBConnectorAccessor и создает DBConnectorAccessor с указанием этого минимально нагруженного DBConnectorReader
	DBConnectorAccessor& GetAccessor();
	
	//Функция "санитизации": при внезапном удалении потока обработчика, поток не успеет вызвать EndAccess(), так?
	//Ну так вот, чтобы все же убить DBConnectorAccessor, мы запоминаем id убитого потока и передаем сюда.
	//manager найдет все DBConnectorAccessor, у которых этот id и удалит их
	bool RemoveAccessorsByThreadId(int thread_id);

	friend class DBConnectorAccessor;
};


