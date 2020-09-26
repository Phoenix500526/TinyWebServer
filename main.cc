#include "Net/WebServer.h"
#include <unistd.h>
using namespace std;

int main(void){
    int port, sqlPort, ConnectionNum, threadNum;
    string sqlUser, sqlPwd, DBName;

    try{
        Config Settings("./config.ini");
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
    WebServer server(port, 5000, sqlPort, sqlUser, sqlPwd,DBName, ConnectionNum, threadNum );
    server.Start();
    return 0;
}