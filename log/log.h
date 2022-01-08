#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <mutex>
#include <string>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include <pthread.h>
#include <thread>
#include "blockqueue.h"

using namespace std;

class Log
{
public:

    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread()
    {
        Log::get_instance()->async_write_log();
    }
    // 参数分别为日志路径、是否关闭log、最大logsize、最大行数、阻塞队列长度（默认为0，非0则为异步）
    bool init(const char *file_path  = "./server_log", int close_log = 0, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

private:
    Log();
    virtual ~Log();
    void *async_write_log()
    {
        string single_log = "";
        //从阻塞队列中取出一个日志string，写入文件
        while (deque_->pop(single_log))
        {
            lock_guard<mutex> locker(m_mtx);
            fputs(single_log.c_str(), m_fp);            
        }
    }

private:
    const char *dir_name;     //路径名
    const char *log_name;     //log文件名
    int m_split_lines;      //日志最大行数
    int m_log_buf_size;     //日志缓冲区大小
    long long m_count;      //日志行数记录
    int m_today;            //因为按天分类,记录当前时间是那一天
    FILE *m_fp;             //打开log的文件指针
    char *m_buf;   
    //BlockDeque<string> *m_log_queue; //阻塞队列
    //BlockDeque<string> deque_;
    //thread writeThread_;
    bool m_is_async;                  //是否同步标志位
    int m_close_log;        //关闭日志
    
    std::unique_ptr<BlockDeque<std::string>> deque_; //阻塞队列
    std::unique_ptr<std::thread> writeThread_;  //写线程
    std::mutex m_mtx;       //互斥锁
    
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif
