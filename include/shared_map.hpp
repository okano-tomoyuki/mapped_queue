#ifndef UTILITY_SHARED_MAP_HPP
#define UTILITY_SHARED_MAP_HPP

#include <mutex>
#include <unordered_map>
#include <memory>
#include <string>
#include <typeinfo>
#include <stdexcept>

class SharedMap {
public:
    template<typename T>
    static std::shared_ptr<T> acquire(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        auto it = table_.find(key);
        if (it != table_.end()) {
            if (it->second.type != &typeid(T)) {
                throw std::runtime_error("Type mismatch: Key '" + key + "' is already used with a different type");
            }            
            if (auto existing = it->second.ptr.lock()) {
                return std::static_pointer_cast<T>(existing);
            }
        }
        
        auto ptr = std::shared_ptr<T>(new T, Deleter{key});
        table_[key] = Value{ptr, &typeid(T)};
        return ptr;
    }

private:
    struct Value {
        std::weak_ptr<void> ptr;
        const std::type_info* type;
    };
    struct Deleter {
        const std::string key;
        
        template<typename T>
        void operator()(T* ptr) {
            std::lock_guard<std::mutex> lock(SharedMap::mtx_);
            SharedMap::table_.erase(key);
            delete ptr;
        }
    };
    static std::mutex mtx_;
    static std::unordered_map<std::string, Value> table_;
};

std::mutex SharedMap::mtx_;
std::unordered_map<std::string, SharedMap::Value> SharedMap::table_;

#endif  // UTILITY_SHARED_MAP_HPP
