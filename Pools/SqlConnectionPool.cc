#include "Pools/SqlConnectionPool.h"

using namespace std;

connection_pool::connection_pool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}

//构造初始化
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn)
{
	m_url = url;
	m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			LOG_FATAL << "MySQL Init Error";
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			LOG_FATAL << "MySQL Get Real Connection Error";
		}
		connList.push_back(con);
		++m_FreeConn;
	}

	m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection()
{
	MYSQL *con = NULL;

	if (0 == connList.size())
		return NULL;
	UniqueLock lck(m_mtx);
	while(m_FreeConn <= 0){
		m_cond.wait(lck);
	}
	assert(m_FreeConn > 0);
	con = connList.front();
	connList.pop_front();
	--m_FreeConn;
	++m_CurConn;
	return con;
}

//释放当前使用的连接【即将线程归还给线程池】
bool connection_pool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;
	UniqueLock lck(m_mtx);
	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;
	m_cond.notify_one();
	return true;
}

//销毁数据库连接池
void connection_pool::DestroyPool()
{
	UniqueLock lck(m_mtx);
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}
}

//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}

connection_pool::~connection_pool()
{
	DestroyPool();
}
