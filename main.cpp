#include "shared_map.hpp"
#include "concurrent_queue.hpp"

#include <iostream>
#include <chrono>
#include <thread>
 
constexpr char ARGUMENT_QUEUE_NAME[16]  = "ARGUMENT_QUEUE";
constexpr char RESULT_QUEUE_NAME[16]    =   "RESULT_QUEUE";
constexpr int  THREAD1_INTERVAL_MSEC    =              100;
constexpr int  THREAD2_INTERVAL_MSEC    =               10;
constexpr int  MAXIMUM_ITERATION_COUNT  =               30;

struct Argument {
    int         count;
    double      height;
    double      weight;
    std::string first_name;
    std::string family_name;
};

struct Result {
    double      bmi;
    std::string full_name;
};

void producer_func() {
    auto argument_queue = SharedMap::acquire<ConcurrentQueue<Argument>>(ARGUMENT_QUEUE_NAME);
    auto result_queue   = SharedMap::acquire<ConcurrentQueue<Result>>(RESULT_QUEUE_NAME);
    
    for (int i = 0; i < MAXIMUM_ITERATION_COUNT; ++i) {
        Argument argument = {};
        argument.count          = i + 1;
        argument.height         = 160.0 + i * 0.1;
        argument.weight         =  60.0 + i * 0.2;
        argument.first_name     = std::to_string(i * 2);
        argument.family_name    = std::to_string(i);
        argument_queue->push(argument);

        Result result = {};
        while (!result_queue->try_pop(result)) {
            std::this_thread::yield();
        }

        std::cout << "[#1]"
                  << " full-name=" << result.full_name
                  << " bmi=" << result.bmi
                  << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD1_INTERVAL_MSEC));
    }
}

void consumer_func() {
    auto argument_queue = SharedMap::acquire<ConcurrentQueue<Argument>>(ARGUMENT_QUEUE_NAME);
    auto result_queue   = SharedMap::acquire<ConcurrentQueue<Result>>(RESULT_QUEUE_NAME);
    
    Argument argument = {};
    while (argument.count < MAXIMUM_ITERATION_COUNT) {
        while (!argument_queue->try_pop(argument)) {
            std::this_thread::yield();
        }

        std::cout << "[#2]"
                  << " first-name="  << argument.first_name
                  << " family-name=" << argument.family_name
                  << " height="      << argument.height
                  << " weight="      << argument.weight
                  << std::endl;

        Result result = {};
        result.full_name = argument.family_name + '-' + argument.first_name;
        result.bmi       = argument.weight / (argument.height * argument.height * 0.0001);
        result_queue->push(result);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD2_INTERVAL_MSEC));
    }    
}

int main() {
    auto producer_thread = std::thread{[](){ producer_func(); }};
    auto consumer_thread = std::thread{[](){ consumer_func(); }};
 
    producer_thread.join();
    consumer_thread.join();
}
