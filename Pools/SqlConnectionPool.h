#ifndef TINYWEBSERVER_CONNECTION_POOL_H
#define TINYWEBSERVER_CONNECTION_POOL_H

#include <list>
#include <mysql/mysql.h>
#include <string>
#include "Tools/Mutex.h"
#include "Tools/Condition.h"
#include "Logger/Logger.h"

class connection_pool
{
public:
	MYSQL *GetConnection();				 //获取数据库连接
	bool ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接

	//单例模式
	static connection_pool *GetInstance();

	void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn); 

private:
	connection_pool();
	~connection_pool();

	int m_MaxConn;  //最大连接数
	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
	Mutex m_mtx;
	Condition m_cond;
	std::list<MYSQL *> connList; //连接池

public:
	std::string m_url;			 //主机地址
	std::string m_Port;		 //数据库端口号
	std::string m_User;		 //登陆数据库用户名
	std::string m_PassWord;	 //登陆数据库密码
	std::string m_DatabaseName; //使用数据库名
};

class connectionRAII{

public:
	connectionRAII(MYSQL **con, connection_pool *connPool):conRAII(connPool->GetConnection()), poolRAII(connPool){
		*con = conRAII;
	}
	~connectionRAII(){
		poolRAII->ReleaseConnection(conRAII);
	}
	
private:
	MYSQL *conRAII;
	connection_pool *poolRAII;
};

//防止用到匿名对象，导致刚申请链接马上被释放。
#define connectionRAII(x, y) error "Missing connection object name"

#endif
