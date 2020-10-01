#include "Pools/SqlConnectionPool.h"

using namespace std;

DAOPtr DAOFactory(std::string const& DBType, std::string const& url, 
        std::string const& User, std::string const& PassWord,
        std::string const& DBName, int Port){
    DAOPtr obj(nullptr);
    if(DBType == "MYSQL"){
        obj.reset(new MYSQLDAO(url, User, PassWord, DBName, Port)); 
    }
    return obj;
}

ConnectionPool::ConnectionPool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

ConnectionPool *ConnectionPool::GetInstance()
{
	static ConnectionPool connPool;
	return &connPool;
}

//构造初始化
void ConnectionPool::init(string const& DBType, string const& url, 
		string const& User, string const& PassWord, string const& DBName, 
		int DBPort, int MaxConn)
{
	m_url = url;
	m_Port = DBPort;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;

	for (int i = 0; i < MaxConn; i++)
	{
		connList.push_back(DAOFactory(DBType, url, User, PassWord, DBName, DBPort));
		++m_FreeConn;
	}

	m_MaxConn = m_FreeConn;
}

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
DAOPtr ConnectionPool::GetConnection()
{
	if (connList.empty())
		return nullptr;
	UniqueLock lck(m_mtx);
	while(m_FreeConn <= 0){
		m_cond.wait(lck);
	}
	assert(m_FreeConn > 0);
	DAOPtr con = std::move(connList.front());
	connList.pop_front();
	--m_FreeConn;
	++m_CurConn;
	return con;
}

//释放当前使用的连接【即将线程归还给线程池】
bool ConnectionPool::ReleaseConnection(DAOPtr& con)
{
	if (nullptr == con)
		return false;
	UniqueLock lck(m_mtx);
	connList.push_back(std::move(con));
	++m_FreeConn;
	--m_CurConn;
	m_cond.notify_one();
	return true;
}

//销毁数据库连接池
void ConnectionPool::DestroyPool()
{
	UniqueLock lck(m_mtx);
	connList.clear();
}

//当前空闲的连接数
int ConnectionPool::GetFreeConn()
{
	return this->m_FreeConn;
}

ConnectionPool::~ConnectionPool()
{
	DestroyPool();
}
