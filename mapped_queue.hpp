#ifndef UTILITY_MAPPED_QUEUE_HPP
#define UTILITY_MAPPED_QUEUE_HPP

#include <mutex>
#include <queue>
#include <string>
#include <stdexcept>
#include <unordered_map>

template<typename T, typename U>
class MappedQueue
{
public:
    ~MappedQueue()
    {
        if (queue_ != nullptr)
            detach();
    }

    void create(const char* key)
    {
        create(std::string(key));
    }
 
    void attach(const char* key)
    {
        attach(std::string(key));
    }

    void create(const T& key)
    {
        auto& table = get_table();
        if (table.count(key) > 0)
        {}
       
        std::lock_guard<std::mutex> lock(table[key].mutex);
        table[key].key      = key;
        table[key].count    = 0;
    }
 
    void attach(const T& key)
    {
        queue_ = &get_table().at(key);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        queue_->count++;
    }
 
    void detach()
    {
        std::lock_guard<std::mutex> lock(queue_->mutex);
        queue_->count--;
        if (queue_->count == 0)
        {
            get_table().erase(queue_->key);
        }
        queue_ = nullptr;
    }
 
    bool empty() const
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex); 
        return queue_->data.empty();
    }
 
    void clear() const
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        while(!queue_->data.empty())
        {
            queue_->data.pop();
        }
    }
 
    size_t size() const
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        return queue_->data.size();
    }
 
    void push(const U& val)
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        queue_->data.push(val);
    }
 
    void pop()
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        queue_->data.pop();
    }
 
    U front() const
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        return queue_->data.front();
    }
 
    U back() const
    {
        check_attached(queue_, __func__);
        std::lock_guard<std::mutex> lock(queue_->mutex);
        return queue_->data.back();
    }

private:

    struct Queue
    {
        std::mutex      mutex;
        size_t          count;
        T               key;
        std::queue<U>   data;
    };

    Queue* queue_ = nullptr;

    static void check_attached(Queue* queue, const char* func_name)
    {
        if (queue == nullptr)
            throw std::runtime_error("NamedQueue '" + std::string(func_name) + std::string("' method was failed! Queue is not attached."));
    }

    static std::unordered_map<std::string, Queue>& get_table()
    {
        static std::unordered_map<std::string, Queue> table;
        return table;
    }
};

template<typename U>
using NamedQueue = MappedQueue<std::string, U>;

#endif // UTILITY_MAPPED_QUEUE_HPP
