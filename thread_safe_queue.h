#ifndef __THREAD_SAFE_QUEUE_H__
#define __THREAD_SAFE_QUEUE_H__
#include <mutex>
#include <queue>
template <typename K> class myqueue {
private:
  std::queue<K> myqueue;
  std::mutex mtx;

public:
  K front;
  bool empty() { return myqueue.empty(); }
  K pop() {
    std::lock_guard<std::mutex> lck(mtx);
    K res = myqueue.front;
    myqueue.pop();
    return res;
  }
  void push(K value) { myqueue.push(value); }
};
#endif
