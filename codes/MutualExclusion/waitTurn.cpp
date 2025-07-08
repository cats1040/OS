#include <iostream>
#include <thread>

int turn = 0;

class thread1
{
public:
    void non_critical_section()
    {
        int a = 0;
        while (++a < 10)
            ;
    }

    void operator()(int &x)
    {
        while (true)
        {
            while (turn != 0)
                ;
            x += 1;
            std::cout << "Thread 1 " << x << std::endl;
            turn = 1;
            non_critical_section();
        }
    }
};

class thread2
{
public:
    void non_critical_section()
    {
        int a = 0;
        while (++a < 10000000)
            ;
    }

    void operator()(int &x)
    {
        while (true)
        {
            while (turn != 1)
                ;
            x += 1;
            std::cout << "Thread 2 " << x << std::endl;
            turn = 0;
            non_critical_section();
        }
    }
};

int main()
{
    int x = 0;

    thread1 obj1;
    thread2 obj2;

    std::thread t1(std::ref(obj1), std::ref(x));
    std::thread t2(std::ref(obj2), std::ref(x));

    t1.join();
    t2.join();

    return 0;
}