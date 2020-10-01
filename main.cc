#include "Net/WebServer.h"
#include <unistd.h>
#include <sys/resource.h>

using namespace std;

AsyncLogging* g_asyncLog = NULL;
void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}

int main(int argc, char* argv[]){
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000*1024*1024;
        rlimit rl = { 2*kOneGB, 2*kOneGB };
        setrlimit(RLIMIT_AS, &rl);
    }
    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    AsyncLogging log(basename(name));
    log.start();
    g_asyncLog = &log;
    Logger::setOutput(asyncOutput);

    int port, sqlPort, ConnectionNum, threadNum;
    string sqlUser, sqlPwd, DBName, DBType;

    try{
        Config Settings("./config.ini");
        Settings.Read("DBType", DBType);
        Settings.Read("port",port);
        Settings.Read("sqlPort",sqlPort);
        Settings.Read("sqlUser",sqlUser);
        Settings.Read("sqlPwd",sqlPwd);
        Settings.Read("DBName",DBName);
        Settings.Read("ConnectionNum",ConnectionNum);
        Settings.Read("threadNum",threadNum);
    }catch(File_Not_Found& e){
        cerr << "Cannot Find the Configuration File:" << e.what() << endl;
        exit(1);
    }catch(Key_Not_Found& e){
        cerr << "Cannot Find the Keyword:" << e.what() << endl;
        exit(1);
    }
    WebServer server(DBType, port, 5000, sqlPort, sqlUser, sqlPwd,DBName, ConnectionNum, threadNum );
    server.Start();
    return 0;
}