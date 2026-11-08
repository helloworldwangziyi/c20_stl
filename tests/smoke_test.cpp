#include <iostream>
#include <c20_stl/type_traits.hpp>
#include <cassert>

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
}

c20_stl::test_counted_object test_template_object()
{
    return c20_stl::test_counted_object{};
}

int main(){
    std::cout << "this is my first time to study stl" << std::endl;
    std::cout << "heollo world" << std::endl;

    auto t = test_template_object();
    assert(t.constructions == 1);
    return 0;
}