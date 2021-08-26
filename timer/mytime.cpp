#include "myTimer.h"

// 删除指定位置的结点
void MyTimer::del_(size_t index)
{
    
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void MyTimer::SwapNode_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void MyTimer::siftup_(size_t i)
{
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);
        i = j;        
        j = (i - 1) / 2;
    }
}

bool MyTimer::siftdown_(size_t index, size_t n)
{
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n<=heap_.size());

    size_t i = index;
    //右节点
    size_t j= i*2 + 1;

    while(j<n)
    {
        //找更小
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i,j);
        i=j;
        j=i*2+1;
    }

    return i > index;


}

//-----------------------------------------------------------------------//

// 添加指定id、超时时间、回调函数 的结点
void MyTimer::add(int id, int timeout, const TimeoutCallBack& cb)
{
    assert(id>=0);
    size_t i;
    //堆中未有该节点
    if(ref_.count(id) == 0)
    {
        i=heap_.size();
        ref_[id]=i;
        heap_.push_back({id,Clock::now()+MS(timeout),cb});
        siftup_(i);
    }
    else
    {
        i=ref_[id];
        heap_[i].expires = Clock::now()+MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) 
        {
            siftup_(i);
        }
    }
    
}

// 删除指定id结点，并触发回调函数 
void MyTimer::doWork(int id)
{
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode t = heap_[i];

    t.cb();
    del_(i);

}

// 调整指定id节点的timeout
void MyTimer::adjust(int id, int timeout)
{
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    siftdown_(ref_[id], heap_.size());
}

void MyTimer::clear()
{
    ref_.clear();
    heap_.clear();
}

void MyTimer::tick()
{
    if(heap_.empty())
        return;
    
    while(!heap_.empty())
    {
        TimerNode t = heap_.front();
        if(std::chrono::duration_cast<MS>(t.expires - Clock::now()).count()>0)
            break;
        t.cb();
        pop();
    }
}

void MyTimer::pop()
{
    assert(!heap_.empty());
    del_(0);   
}

int MyTimer::GetNextTick()
{
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}