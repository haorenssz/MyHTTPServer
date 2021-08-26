
#include "log.h"
//using namespace std::;

Log::Log()
{
    m_count = 0;
    m_is_async = false;
    writeThread_ = nullptr;
    deque_ = nullptr;
    m_today = 0;
    m_fp = nullptr;
}

Log::~Log()
{
    if (m_fp != NULL)
    {
        fclose(m_fp);
    }
}


bool Log::init(const char *file_path, int close_log, int log_buf_size, int split_lines, int max_queue_size)
{
    //如果设置了max_queue_size,则设置为异步
    if (max_queue_size > 0)
    {
        m_is_async = true;
        if(!deque_)
        {
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = move(newDeque);

            //flush_log_thread为回调函数,这里表示创建线程异步写日志
            std::unique_ptr<std::thread> NewThread(new thread(flush_log_thread));
            writeThread_ = move(NewThread);
        }
        

    }
    else 
        m_is_async = false;
    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_split_lines = split_lines;

    //char* buffer
    m_buf = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);

    //my_time
    time_t t = time(NULL);
    struct tm *sys_time = localtime(&t);
    struct tm my_time = *sys_time;
    m_today = my_time.tm_mday;

    char log_file_name[256] = {0};
    dir_name = file_path;
    log_name = "LOG.txt";
    //日志文件命名格式：.../.../2021_01_01_LOG.txt
    snprintf(log_file_name, 255, "%s/%04d_%02d_%02d%s", 
            dir_name, my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, log_name);

    {
        lock_guard<mutex> locker(m_mtx);
        if(m_fp) { 
            flush();
            fclose(m_fp); 
        }

        m_fp = fopen(log_file_name, "a");
        if(m_fp == nullptr) {
            mkdir(dir_name, 0777);
            m_fp = fopen(log_file_name, "a");
        } 
        assert(m_fp != nullptr);
    }
        
}

void Log::write_log(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t_sec = now.tv_sec;
    struct tm *sys_tm = localtime(&t_sec);
    struct tm my_tm = *sys_tm;
    
    //压入不同等级log代表类型
    char kind[16] = {0};
    switch (level)
    {
    case 0:
        strcpy(kind, "[debug]:");
        break;
    case 1:
        strcpy(kind, "[info]:");
        break;
    case 2:
        strcpy(kind, "[warn]:");
        break;
    case 3:
        strcpy(kind, "[erro]:");
        break;
    default:
        strcpy(kind, "[info]:");
        break;
    }
    

    unique_lock<mutex> locker(m_mtx);
    m_count++;
    //
    if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
    {
        unique_lock<mutex> locker(m_mtx);
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if (m_today != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_lines);
        }
        m_fp = fopen(new_log, "a");

    }

    va_list valst;
    va_start(valst, format);

    string log_str;
    {
        unique_lock<mutex> locker(m_mtx);
        int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                        my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                        my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, kind);
        
        int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);

        m_buf[n + m] = '\n';
        m_buf[n + m + 1] = '\0';
        log_str = m_buf;
    }

    if (m_is_async && !m_log_queue->full())
    {
        m_log_queue->push_back(log_str);
    }
    else
    {
        unique_lock<mutex> locker(m_mtx);
        fputs(log_str.c_str(), m_fp);        
    }

    va_end(valst);
}


void Log::flush(void)
{
    lock_guard<mutex> locker(m_mtx);
    fflush(m_fp);
}