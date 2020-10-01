#ifndef TINYWEBSERVER_CONNECTION_POOL_H
#define TINYWEBSERVER_CONNECTION_POOL_H

#include <list>
#include <mysql/mysql.h>
#include <string>
#include "Tools/Mutex.h"
#include "Tools/Condition.h"
#include "Logger/Logger.h"
#include "Pools/DAO.h"

using DAOPtr = std::unique_ptr<DAO>;

class ConnectionPool
{
public:
	DAOPtr GetConnection();					//获取数据库连接
	bool ReleaseConnection(DAOPtr& conn); 	//释放连接
	int GetFreeConn();					 	//获取连接
	void DestroyPool();					 	//销毁所有连接

	//单例模式
	static ConnectionPool *GetInstance();

	void init(std::string const& DBType, std::string const& url, 
			std::string const& User, std::string const& PassWord, 
			std::string const& DataBaseName, int Port, int MaxConn); 

private:
	ConnectionPool();
	~ConnectionPool();

	int m_MaxConn;  //最大连接数
	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
	Mutex m_mtx;
	Condition m_cond;
	std::list<DAOPtr> connList;

public:
	std::string m_url;			//主机地址
	int m_Port;		 			//数据库端口号
	std::string m_User;		 	//登陆数据库用户名
	std::string m_PassWord;	 	//登陆数据库密码
	std::string m_DatabaseName; //使用数据库名
};

class connectionRAII{
public:
	connectionRAII(ConnectionPool *connPool):poolRAII(connPool), connRAII(connPool->GetConnection()){}
	~connectionRAII(){
		poolRAII->ReleaseConnection(connRAII);
	}
	bool Insert(std::string const& name, std::string const& pwd){
		return connRAII->Insert(name, pwd);
	}
	bool Select(std::unordered_map<std::string, std::string>& table){
		return connRAII->Select(table);
	}
	bool IsValid() const{
		return connRAII != nullptr;
	}
private:
	ConnectionPool *poolRAII;
	DAOPtr connRAII;
	
};

//防止用到匿名对象，导致刚申请链接马上被释放。
#define connectionRAII(x, y) error "Missing connection object name"

#endif
