#include "mapped_queue.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>
#include <cstring>
 
constexpr char ARGUMENT_QUEUE_NAME[16]  = "ARGUMENT_QUEUE";
constexpr char RESULT_QUEUE_NAME[16]    =   "RESULT_QUEUE";
constexpr int  THREAD1_INTERVAL_MSEC    =              100;
constexpr int  THREAD2_INTERVAL_MSEC    =               10;
constexpr int  MAXIMUM_ITERATION_COUNT  =               30;

struct Argument
{
    int         count;
    double      height;
    double      weight;
    std::string first_name;
    std::string family_name;
};

struct Result
{
    double      bmi;
    std::string full_name;
};

// run on productor thread 
void producer_func();

// run on consumer thread
void consumer_func();

int main()
{ 
    auto argument_queue = NamedQueue<Argument>();
    auto result_queue   = NamedQueue<Result>();

    argument_queue.create(ARGUMENT_QUEUE_NAME);    
    result_queue.create(RESULT_QUEUE_NAME);

    auto producer_thread = std::thread{[](){ producer_func(); }};
    auto consumer_thread = std::thread{[](){ consumer_func(); }};
 
    producer_thread.join();
    consumer_thread.join();
}

void producer_func()
{
    auto argument_queue = NamedQueue<Argument>();
    auto result_queue   = NamedQueue<Result>();
    
    argument_queue.attach(ARGUMENT_QUEUE_NAME);
    result_queue.attach(RESULT_QUEUE_NAME);
    
    Argument argument;
    Result   result;
    for (int i = 1; i <= MAXIMUM_ITERATION_COUNT; i++)
    {
        argument.count          = i;
        argument.height         = 160.0 + i * 0.1;
        argument.weight         =  60.0 + i * 0.2;
        argument.family_name    = std::string("person-" + std::to_string(i));
        argument_queue.push(argument);

        while (!result_queue.empty())
        {
            result = result_queue.front();
            std::cout << "full name : " << result.full_name << " bmi : " << result.bmi << std::endl;
            result_queue.pop();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD1_INTERVAL_MSEC));
    }
}

void consumer_func()
{
    auto argument_queue = NamedQueue<Argument>();
    auto result_queue   = NamedQueue<Result>();
    
    argument_queue.attach(ARGUMENT_QUEUE_NAME);
    result_queue.attach(RESULT_QUEUE_NAME);
    
    Argument argument;
    Result   result;

    while (true)
    {
        while (!argument_queue.empty())
        {
            argument    = argument_queue.front();
            std::cout   << "first name : "  << argument.first_name << " family name : " << argument.family_name 
                        << "height : "      << argument.height     << " weight : "     << argument.weight << std::endl;

            argument_queue.pop();

            result.full_name = argument.family_name + '-' + argument.first_name;
            result.bmi       = argument.weight / (argument.height * argument.height * 0.0001);

            result_queue.push(result);
        }

        if (argument.count == MAXIMUM_ITERATION_COUNT)
            break;
            
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD2_INTERVAL_MSEC));
    }    
}