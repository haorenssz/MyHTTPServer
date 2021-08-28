
#ifndef WEBSERVER_H
#define WEBSERVER_H



#include "../epoll/epoller.h"
#include "../log/log.h"
#include "../timer/myTimer.h"
#include "../mysql/sql_connection_pool.h"
#include "../threadpool/threadpool.h"
//#include "../http/xxx"

class WebServer {
public:
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog);//....

    ~WebServer();
    void Start();

private:
    bool InitSocket_(); 
    
};


#endif //WEBSERVER_H