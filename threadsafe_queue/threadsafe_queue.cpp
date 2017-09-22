#include<memory>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<chrono>
#include<iostream>
using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        shared_ptr<T> _data;
        unique_ptr<node> _next;
    };
    unique_ptr<node> _head;
    node* _tail;
    mutex _headLock, _tailLock;
    condition_variable _cv;

    node* get_tail()
    {
        lock_guard<mutex> lock(_tailLock);
        return _tail;
    }

public:
    threadsafe_queue() :_head(new node), _tail(_head.get()) { };
    threadsafe_queue(const threadsafe_queue&) = delete;
    threadsafe_queue& operator=(const threadsafe_queue&) = delete;

    shared_ptr<T> try_pop();
    bool try_pop(T& value);

    shared_ptr<T> wait_and_pop();
    //void wait_and_pop(T& value);

    void push(T value);
    bool empty();
};

template<typename T>
void threadsafe_queue<T>::push(T value)
{
    shared_ptr<T> new_value = make_shared<T>(move(value));
    unique_ptr<node> p(new node);
    {
        lock_guard<mutex> lock(_tailLock);
        node* new_tail = p.get();
        _tail->_data = new_value;
        _tail->_next = move(p);
        _tail = new_tail;
    }
    _cv.notify_one();
}

template<typename T>
shared_ptr<T> threadsafe_queue<T>::try_pop()
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
    lock_guard<mutex> lock(_headLock);
    if (_head.get() == get_tail())
        return false;
    unique_ptr<node> old_head = move(_head);
    _head = move(old_head->_next);
    res = *old_head->_data;
    return true;
}

template<typename T>
shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    unique_lock<mutex> head_lock(_headLock);
    _cv.wait(head_lock, [&]{return _head.get() != get_tail(); });
    unique_ptr<node> old_head = move(_head);
    _head = move(old_head->_next);
    return old_head->_data;
}

template<typename T>
bool threadsafe_queue<T>::empty()
{
    lock_guard<mutex> headLock(_headLock);
    return _head.get() == get_tail();
}

int main()
{
    threadsafe_queue<int> queue;
    queue.push(2);
    auto res = queue.try_pop();
    cout << *res << endl;
    res = queue.try_pop();
    if (!res)
        cout << "empty" << endl;
    if (queue.empty())
        cout << "empty" << endl;
    queue.push(3);
    int a = 0;
    queue.try_pop(a);
    cout << a << endl;
    thread thread1([&]{ this_thread::sleep_for(chrono::seconds(5)); queue.push(4); });
    res = queue.wait_and_pop();
    cout << *res << endl;
    thread1.join();
    system("pause");
    return 0;
}
