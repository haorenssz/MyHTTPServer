#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__


#ifndef WEBSERVER_H
#define WEBSERVER_H



#include "../epoll/epoller.h"
#include "../log/log.h"
#include "../timer/myTimer.h"
#include "../mysqlconn/sql_connection_pool.h"
#include "../threadpool/threadpool.h"
#include "../http_conn/http_conn.h"
//#include "../http/xxx"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer {
public:
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logQueSize);//....

    ~WebServer();

    void Start();
    
private:
    bool InitSocket_(); 
    void InitEventMode_(int trigMode);

    void DealListen_();
    void DealWrite_(http_conn* client);
    void DealRead_(http_conn* client);

    void OnRead_(http_conn* client);
    void OnWrite_(http_conn* client);
    void OnProcess(http_conn* client);


    void AddClient_(int fd, sockaddr_in addr);
    void CloseConn_(http_conn* client);

    void SendError_(int fd, const char*info);
    void ExtentTime_(http_conn* client);

    static int SetFdNonblock(int fd);
private:
    //基础
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipefd[2];//signal
    int m_epollfd;
    http_conn *users;

    //线程池相关
    //threadpool<http_conn> *m_pool;
    int m_thread_num;
    
    //数据库相关
    connection_pool *m_connPool;
    string m_user;         //登陆数据库用户名
    string m_passWord;     //登陆数据库密码
    string m_databaseName; //使用数据库名
    int m_sql_num;
    //epoll_event相关

    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;
    int m_Timeout;

    //定时器相关
    std::unique_ptr<MyTimer> timer_;

    uint32_t listenEvent_;
    uint32_t connEvent_;


    
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, http_conn> users_;
    
};


#endif //WEBSERVER_H
#endif // __WEBSERVER_H__