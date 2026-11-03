#include <cassert>
#include <memory>
#include <type_traits>
#include <c20_stl/memory.hpp>

namespace c20_stl{

struct test_counted_object {
    static inline int constructions = 0;
    static inline int destructions = 0;

    int value;

    test_counted_object()
        : value(-1){
        ++constructions;
    }

    explicit test_counted_object(int v)
        : value(v){
        ++constructions;
    }

    ~test_counted_object(){
        ++destructions;
    }
};

void test_allocator(){
    static_assert(std::is_same_v<allocator<int>::value_type, int>);
    static_assert(std::is_convertible_v<allocator<int>, allocator<double>>);

    allocator<int> int_allocator;
    int* int_storage = int_allocator.allocate(3);
    assert(int_storage != nullptr);
    int_allocator.deallocate(int_storage, 3);

    allocator<test_counted_object> allocator;
    test_counted_object* storage = allocator.allocate(2);
    assert(storage != nullptr);
    assert(test_counted_object::constructions == 0);
    allocator.deallocate(storage, 2);
    assert(test_counted_object::destructions == 0);

    std::allocator<test_counted_object> std_allocator;
    test_counted_object* std_storage = std_allocator.allocate(2);
    assert(std_storage != nullptr);
    assert(test_counted_object::constructions == 0);
    std_allocator.deallocate(std_storage, 2);
    assert(test_counted_object::destructions == 0);
}

void test_construct_destroy(){
    test_counted_object::constructions = 0;
    test_counted_object::destructions = 0;

    allocator<test_counted_object> allocator;
    test_counted_object* obj = allocator.allocate(1);
    assert(obj != nullptr);
    assert(test_counted_object::constructions == 0);
    assert(test_counted_object::destructions == 0);

    test_counted_object* result = c20_stl::construct_at(obj, 42);
    assert(result == obj);
    assert(obj->value == 42);
    assert(test_counted_object::constructions == 1);
    assert(test_counted_object::destructions == 0);

    c20_stl::destroy_at(obj);
    assert(test_counted_object::constructions == 1);
    assert(test_counted_object::destructions == 1);

    allocator.deallocate(obj, 1);
    assert(test_counted_object::constructions == 1);
    assert(test_counted_object::destructions == 1);

    std::allocator<test_counted_object> std_allocator;
    test_counted_object* std_obj = std_allocator.allocate(1);
    test_counted_object* std_result = std::construct_at(std_obj, 7);
    assert(std_result == std_obj);
    assert(std_obj->value == 7);
    assert(test_counted_object::constructions == 2);
    assert(test_counted_object::destructions == 1);
    std::destroy_at(std_obj);
    assert(test_counted_object::constructions == 2);
    assert(test_counted_object::destructions == 2);
    std_allocator.deallocate(std_obj, 1);
    assert(test_counted_object::constructions == 2);
    assert(test_counted_object::destructions == 2);
}

}

int main(){
    c20_stl::test_allocator();
    c20_stl::test_construct_destroy();
}
