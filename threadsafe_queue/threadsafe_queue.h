#ifndef _THREAD_SAFE_QUEUE_
#define _THREAD_SAFE_QUEUE_
#include<memory>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<chrono>
namespace lyf{

    template<typename T>
    class threadsafe_queue
    {
    private:
        struct node
        {
            std::shared_ptr<T> _data;
            std::unique_ptr<node> _next;
        };
        std::unique_ptr<node> _head;
        node* _tail;
        std::mutex _headLock, _tailLock;
        std::condition_variable _cv;

        node* get_tail()
        {
            std::lock_guard<mutex> lock(_tailLock);
            return _tail;
        }

    public:
        threadsafe_queue() :_head(new node), _tail(_head.get()) { };
        threadsafe_queue(const threadsafe_queue&) = delete;
        threadsafe_queue& operator=(const threadsafe_queue&) = delete;

        std::shared_ptr<T> try_pop();
        bool try_pop(T& value);

        std::shared_ptr<T> wait_and_pop();
        void wait_and_pop(T& value);

        void push(T value);
        bool empty();
    };

    template<typename T>
    void threadsafe_queue<T>::push(T value)
    {
        std::shared_ptr<T> new_value = make_shared<T>(move(value));
        std::unique_ptr<node> p(new node);
        {
            std::lock_guard<mutex> lock(_tailLock);
            node* new_tail = p.get();
            _tail->_data = new_value;
            _tail->_next = move(p);
            _tail = new_tail;
        }
        _cv.notify_one();
    }

    template<typename T>
    std::shared_ptr<T> threadsafe_queue<T>::try_pop()
    {
        lock_guard<mutex> lock(_headLock);
        if (_head.get() == get_tail())
            return nullptr;
        unique_ptr<node> old_head = move(_head);
        _head = move(old_head->_next);
        return old_head->_data;
    }

    template<typename T>
    bool threadsafe_queue<T>::try_pop(T& res)
    {
        std::lock_guard<mutex> lock(_headLock);
        if (_head.get() == get_tail())
            return false;
        std::unique_ptr<node> old_head = move(_head);
        _head = move(old_head->_next);
        res = move(*old_head->_data);
        return true;
    }

    template<typename T>
    std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
    {
        std::unique_lock<mutex> head_lock(_headLock);
        _cv.wait(head_lock, [&]{return _head.get() != get_tail(); });
        std::unique_ptr<node> old_head = move(_head);
        _head = move(old_head->_next);
        return old_head->_data;
    }

    template<typename T>
    void threadsafe_queue<T>::wait_and_pop(T& value)
    {
        std::unique_lock<mutex> head_lock(_headLock);
        _cv.wait(head_lock, [&]{return _head.get() != get_tail(); });
        std::unique_ptr<node> old_head = move(_head);
        _head = move(old_head->_next);
        value = move(*old_head->_data);
    }

    template<typename T>
    bool threadsafe_queue<T>::empty()
    {
        std::lock_guard<mutex> headLock(_headLock);
        return _head.get() == get_tail();
    }
}
#endif