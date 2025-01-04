#include "shared_map.hpp"
#include "concurrent_queue.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <string>
#include <cassert>

void test_multiple_producers_consumers() {
    auto queue = SharedMap::acquire<ConcurrentQueue<std::string>>("test_queue");
    
    const int32_t NUM_PRODUCERS = 5;
    const int32_t ITEMS_PER_PRODUCER = 100;
    std::vector<std::thread> producers;
    
    for (int32_t i = 0; i < NUM_PRODUCERS; ++i) {
        producers.push_back(std::thread([&queue, i]() {
            for (int32_t j = 0; j < ITEMS_PER_PRODUCER; ++j) {
                queue->push("Item " + std::to_string(i) + "-" + std::to_string(j));
            }
        }));
    }
    
    const int32_t NUM_CONSUMERS = 3;
    std::vector<std::thread> consumers;
    std::atomic<int> total_consumed{0};
    
    for (int32_t i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.push_back(std::thread([&queue, &total_consumed]() {
            while (total_consumed.load() < NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
                std::string item;
                if (queue->try_pop(item)) {
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                }
                std::this_thread::yield();
            }
        }));
    }
    
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    assert(total_consumed.load() == NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    assert(queue->empty());
    std::cout << "test_multiple_producers_consumers: OK" << std::endl;
}

void test_shared_queue() {
    auto queue1 = SharedMap::acquire<ConcurrentQueue<int32_t>>("shared_queue");
    auto queue2 = SharedMap::acquire<ConcurrentQueue<int32_t>>("shared_queue");
    
    assert(queue1 == queue2);
    
    queue1->push(42);
    int32_t value = queue2->front();
    queue2->pop();
    
    assert(value == 42);
    assert(queue1->empty());
    std::cout << "test_shared_queue: OK" << std::endl;
}

int main() {
    test_multiple_producers_consumers();
    test_shared_queue();
    return 0;
}
