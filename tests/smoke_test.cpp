#include <iostream>
#include <c20_stl/type_traits.hpp>
#include <cassert>
#include <cstddef>

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

    // 验证指针相减 得到的类别是 std::ptrdiff_t
    int p[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int * p1 = &p[0];
    int * p2 = &p[9];
    static_assert(c20_stl::is_same_v<decltype(p2 - p1), std::ptrdiff_t>);


    return 0;
}