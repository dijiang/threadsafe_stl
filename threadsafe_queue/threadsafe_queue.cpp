#include<memory>
#include<mutex>
#include<condition_variable>
#include<iostream>
using namespace std;

template<typename T>
class threadsafe_queue
{
public:
	threadsafe_queue():_head(new node),_tail(_head.get()) { };
	//shared_ptr<T> try_pop();
	//bool try_pop(T& value);
	//shared_ptr<T> wait_and_pop();
	//void wait_and_pop(T& value);
	void push(T value);
	//bool empty();
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

int main()
{
	threadsafe_queue<int> queue;
	int i = 2;
	queue.push(i);
	return 0;
}
