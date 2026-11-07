#include <cassert>
#include <memory>
#include <type_traits>
#include <c20_stl/memory.hpp>

namespace c20_stl{

struct test_counted_object {
    static inline int constructions = 0;
    static inline int destructions = 0;
    static inline int copies = 0;
    static inline int moves = 0;

    int value;

    test_counted_object()
        : value(-1){
        ++constructions;
    }

    explicit test_counted_object(int v)
        : value(v){
        ++constructions;
    }

    test_counted_object(const test_counted_object& other)
        : value(other.value){
        ++constructions;
        ++copies;
    }

    test_counted_object(test_counted_object&& other) noexcept
        : value(other.value){
        ++constructions;
        ++moves;
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

void test_uninitialized_fill(){
    test_counted_object::constructions = 0;
    test_counted_object::destructions = 0;
    test_counted_object::copies = 0;
    test_counted_object::moves = 0;

    allocator<test_counted_object> allocator;
    test_counted_object* storage = allocator.allocate(3);
    test_counted_object source(7);

    c20_stl::uninitialized_fill(storage, storage, source);
    assert(test_counted_object::constructions == 1);
    assert(test_counted_object::destructions == 0);
    assert(test_counted_object::copies == 0);
    assert(test_counted_object::moves == 0);

    c20_stl::uninitialized_fill(storage, storage + 3, source);
    assert(test_counted_object::constructions == 4);
    assert(test_counted_object::destructions == 0);
    assert(test_counted_object::copies == 3);
    assert(test_counted_object::moves == 0);
    assert(storage[0].value == 7);
    assert(storage[1].value == 7);
    assert(storage[2].value == 7);

    c20_stl::destroy_at(storage + 0);
    c20_stl::destroy_at(storage + 1);
    c20_stl::destroy_at(storage + 2);
    assert(test_counted_object::constructions == 4);
    assert(test_counted_object::destructions == 3);

    allocator.deallocate(storage, 3);
    assert(test_counted_object::constructions == 4);
    assert(test_counted_object::destructions == 3);

    std::allocator<test_counted_object> std_allocator;
    test_counted_object* std_storage = std_allocator.allocate(3);

    std::uninitialized_fill(std_storage, std_storage + 3, source);
    assert(test_counted_object::constructions == 7);
    assert(test_counted_object::destructions == 3);
    assert(test_counted_object::copies == 6);
    assert(test_counted_object::moves == 0);
    assert(std_storage[0].value == 7);
    assert(std_storage[1].value == 7);
    assert(std_storage[2].value == 7);

    std::destroy_at(std_storage + 0);
    std::destroy_at(std_storage + 1);
    std::destroy_at(std_storage + 2);
    assert(test_counted_object::constructions == 7);
    assert(test_counted_object::destructions == 6);

    std_allocator.deallocate(std_storage, 3);
    assert(test_counted_object::constructions == 7);
    assert(test_counted_object::destructions == 6);
    assert(test_counted_object::copies == 6);
    assert(test_counted_object::moves == 0);
}

void test_uninitialized_copy()
{
    static_assert(std::is_same_v<
        decltype(c20_stl::uninitialized_copy(
            static_cast<const test_counted_object*>(nullptr),
            static_cast<const test_counted_object*>(nullptr),
            static_cast<test_counted_object*>(nullptr))),
        test_counted_object*>);

    test_counted_object::constructions = 0;
    test_counted_object::destructions = 0;
    test_counted_object::copies = 0;
    test_counted_object::moves = 0;

    c20_stl::allocator<test_counted_object> obj_allocator;
    std::allocator<test_counted_object> std_allocator;
    test_counted_object* storage = obj_allocator.allocate(10);
    test_counted_object* dest = obj_allocator.allocate(10);
    test_counted_object* std_dest = std_allocator.allocate(10);

    test_counted_object* empty_result =
        c20_stl::uninitialized_copy(storage, storage, dest);
    assert(empty_result == dest);
    assert(test_counted_object::constructions == 0);
    assert(test_counted_object::destructions == 0);
    assert(test_counted_object::copies == 0);
    assert(test_counted_object::moves == 0);

    for (int i = 0; i < 10; i++)
    {
        construct_at(&storage[i], i);
    }
    assert(test_counted_object::constructions == 10);

    int constructions_before = test_counted_object::constructions;
    int copies_before = test_counted_object::copies;
    int moves_before = test_counted_object::moves;

    test_counted_object* result =
        c20_stl::uninitialized_copy(storage, storage + 10, dest);

    const int c20_constructions =
        test_counted_object::constructions - constructions_before;
    const int c20_copies = test_counted_object::copies - copies_before;
    const int c20_moves = test_counted_object::moves - moves_before;

    assert(result == dest + 10);
    assert(c20_constructions == 10);
    assert(c20_copies == 10);
    assert(c20_moves == 0);
    assert(test_counted_object::destructions == 0);

    constructions_before = test_counted_object::constructions;
    copies_before = test_counted_object::copies;
    moves_before = test_counted_object::moves;

    test_counted_object* std_result =
        std::uninitialized_copy(storage, storage + 10, std_dest);

    const int std_constructions =
        test_counted_object::constructions - constructions_before;
    const int std_copies = test_counted_object::copies - copies_before;
    const int std_moves = test_counted_object::moves - moves_before;

    assert(std_result == std_dest + 10);
    assert(std_constructions == c20_constructions);
    assert(std_copies == c20_copies);
    assert(std_moves == c20_moves);

    for (int i = 0; i < 10; i++)
    {
        assert(storage[i].value == i);
        assert(dest[i].value == i);
        assert(dest[i].value == std_dest[i].value);
    }

    for (int i = 0; i < 10; i++)
    {
        c20_stl::destroy_at(&storage[i]);
        c20_stl::destroy_at(&dest[i]);
        std::destroy_at(&std_dest[i]);
    }
    assert(test_counted_object::constructions == 30);
    assert(test_counted_object::destructions == 30);
    assert(test_counted_object::copies == 20);
    assert(test_counted_object::moves == 0);

    obj_allocator.deallocate(storage, 10);
    obj_allocator.deallocate(dest, 10);
    std_allocator.deallocate(std_dest, 10);
}

}

int main(){
    c20_stl::test_allocator();
    c20_stl::test_construct_destroy();
    c20_stl::test_uninitialized_fill();
    c20_stl::test_uninitialized_copy();
}
