#include "webserver.h"
using namespace std;

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logQueSize):
        timer_(new MyTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    //root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    //
    m_port = port;
    m_user = sqlUser;
    m_passWord = sqlPwd;
    m_databaseName = dbName;
    m_sql_num = connPoolNum;
    m_thread_num = threadNum;
    //m_log_write = log_write;
    m_OPT_LINGER = OptLinger;
    m_TRIGMode = trigMode;
    m_Timeout = timeoutMS;
    //m_close_log = close_log;
    //m_actormodel = actor_model;
    http_conn::m_user_count = 0;
    //http_conn::m_real_file = srcDir_;

    InitSocket_();
}

bool WebServer::InitSocket_()
{
    int ret;
    struct sockaddr_in addr;
    if(m_port>65535||m_port<1024)
    {
        LOG_ERROR("Port:%d error!",  m_port);
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    struct linger optLinger = { 0 };
    if(m_OPT_LINGER) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenfd < 0) {
        LOG_ERROR("Create socket error!", m_port);
        return false;
    }

    ret = setsockopt(m_listenfd , SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(m_listenfd);
        LOG_ERROR("Init linger error!", m_port);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(m_listenfd);
        return false;
    }

    ret = bind(m_listenfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", m_port);
        close(m_listenfd);
        return false;
    }

    ret = listen(m_listenfd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", m_port);
        close(m_listenfd);
        return false;
    }
    ret = epoller_->AddFd(m_listenfd,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(m_listenfd);
        return false;
    }
    SetFdNonblock(m_listenfd);
    LOG_INFO("Server port:%d", m_port);
    return true;
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    //http_conn::isET = (connEvent_ & EPOLLET);
}

void WebServer::DealListen_()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    do{
        int fd = accept(m_listenfd,(struct sockaddr *)&addr, &len);
        if(fd<=0)
            return;
        else if(http_conn::m_user_count>=MAX_FD)
        {
            //send error;
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);


    }
    while(listenEvent_& EPOLLET);
}

void WebServer::AddClient_ (int fd, sockaddr_in addr)
{
    assert(fd>0);
    users_[fd].init(fd,addr, m_root, m_CONNTrigmode, m_close_log, m_user, m_passWord, m_databaseName);
    if(m_Timeout>0)
    {
        timer_->add(fd, m_Timeout,std::bind(&WebServer::CloseConn_,this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::CloseConn_(http_conn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->close_conn();
}

void WebServer::SendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::ExtentTime_(http_conn* client) {
    assert(client);
    if(m_Timeout > 0) { timer_->adjust(client->GetFd(), m_Timeout); }
}

void WebServer::OnRead_(http_conn* client) {
    assert(client);
    if (1 == m_actormodel)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        //若监测到读事件，将该事件放入请求队列
        m_pool->append(users + sockfd, 0);

        while (true)
        {
            if (1 == users[sockfd].improv)
            {
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (client->read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(client->get_address()->sin_addr));


        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}