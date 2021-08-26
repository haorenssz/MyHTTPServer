#ifndef HTTPSERVER_TIMER_H
#define HTTPSERVER_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>


typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t) {return expires < t.expires;}

};

class MyTimer {
public:
    MyTimer() { heap_.reserve(64); }

    ~MyTimer() { clear(); }

    void add(int id, int timeout, const TimeoutCallBack& cb);

    void doWork(int id);

    void adjust(int id, int timeout);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);
    
    void siftup_(size_t i);

    bool siftdown_(size_t index, size_t n);

    void SwapNode_(size_t i, size_t j);

private:
    std::vector<TimerNode> heap_;
    //id--下标 对照
    std::unordered_map<int, size_t> ref_;

};

#endif //HTTPSERVER_TIMER_H