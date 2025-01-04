#include "shared_map.hpp"
#include <cassert>
#include <thread>
#include <vector>
#include <iostream>

class TestObject {
public:
    TestObject() : value_(0) {}
    void setValue(int32_t v) { value_ = v; }
    int32_t getValue() const { return value_; }
private:
    int32_t value_;
};

void test_basic_get() {
    auto obj1 = SharedMap::acquire<TestObject>("test1");
    obj1->setValue(42);
    
    auto obj2 = SharedMap::acquire<TestObject>("test1");
    assert(obj2->getValue() == 42);
    assert(obj1 == obj2);
    std::cout << "test_basic_get: OK" << std::endl;
}

void test_multiple_objects() {
    auto obj1 = SharedMap::acquire<TestObject>("test1");
    auto obj2 = SharedMap::acquire<TestObject>("test2");
    
    obj1->setValue(42);
    obj2->setValue(24);
    
    assert(obj1 != obj2);
    assert(obj1->getValue() == 42);
    assert(obj2->getValue() == 24);
    std::cout << "test_multiple_objects: OK" << std::endl;
}

void test_automatic_cleanup() {
    std::weak_ptr<TestObject> weak_obj;
    {
        auto obj = SharedMap::acquire<TestObject>("test_cleanup");
        weak_obj = obj;
        assert(!weak_obj.expired());
    }
    assert(weak_obj.expired());
    
    auto new_obj = SharedMap::acquire<TestObject>("test_cleanup");
    assert(weak_obj.lock() != new_obj);
    std::cout << "test_automatic_cleanup: OK" << std::endl;
}

void test_thread_safety() {
    const int32_t NUM_THREADS = 10;
    std::vector<std::thread> threads;
    
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(std::thread([i]() {
            auto obj = SharedMap::acquire<TestObject>("thread_test");
            obj->setValue(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }));
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "test_thread_safety: OK" << std::endl;
}


void test_edge_cases() {
    // 空のキーでの取得テスト
    auto empty_key_obj = SharedMap::acquire<TestObject>("");
    empty_key_obj->setValue(100);
    auto empty_key_obj2 = SharedMap::acquire<TestObject>("");
    assert(empty_key_obj == empty_key_obj2);
    assert(empty_key_obj2->getValue() == 100);
        
    // 大量のオブジェクト作成と解放のテスト
    std::vector<std::weak_ptr<TestObject>> weak_refs;
    for (int i = 0; i < 1000; ++i) {
        auto obj = SharedMap::acquire<TestObject>("stress_test_" + std::to_string(i));
        weak_refs.push_back(obj);
    }
    
    // メモリリークがないことを確認
    for (const auto& weak_ref : weak_refs) {
        assert(weak_ref.expired());
    }
    
    std::cout << "test_edge_cases: OK" << std::endl;
}

void test_concurrent_different_keys() {
    const int32_t NUM_THREADS = 10;
    std::vector<std::thread> threads;
    
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(std::thread([i]() {
            // 各スレッドが異なるキーでオブジェクトを取得
            std::string key = "concurrent_key_" + std::to_string(i);
            auto obj = SharedMap::acquire<TestObject>(key);
            obj->setValue(i);
            
            // 意図的に競合状態を作る
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            // 同じキーで再度取得して値を確認
            auto obj2 = SharedMap::acquire<TestObject>(key);
            assert(obj == obj2);
            assert(obj2->getValue() == i);
        }));
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "test_concurrent_different_keys: OK" << std::endl;
}

void test_type_safety() {
    struct Type1 { int value = 1; };
    struct Type2 { int value = 2; };
    
    auto obj1 = SharedMap::acquire<Type1>("test_key");
    
    // 同じキーで異なる型を取得しようとすると例外がスローされる
    try {
        auto obj2 = SharedMap::acquire<Type2>("test_key");
        assert(false && "Should throw exception");
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    // 同じ型で同じキーの場合は正常に動作
    auto obj3 = SharedMap::acquire<Type1>("test_key");
    assert(obj1 == obj3);
    
    std::cout << "test_type_safety: OK" << std::endl;
}

int main() {
    test_basic_get();
    test_multiple_objects();
    test_automatic_cleanup();
    test_thread_safety();
    test_edge_cases();
    test_concurrent_different_keys();
    test_type_safety();
    return 0;
}
