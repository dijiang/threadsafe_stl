#include<iostream>
#include"threadsafe_queue.h"
using namespace std;

int main()
{
    lyf::threadsafe_queue<int> queue;
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
    queue.push(5);
    queue.wait_and_pop(a);
    cout << a << endl;


    system("pause");
    return 0;
}