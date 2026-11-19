#include <cassert>
#include <c20_stl/vector.hpp>
#include <c20_stl/type_traits.hpp>
#include <type_traits>
#include <vector>

namespace c20_stl{

void test_vector_type(){
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::value_type, int>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::allocator_type, c20_stl::allocator<int>>);
    static_assert(std::is_same_v<c20_stl::vector<int>::value_type,
                                 std::vector<int>::value_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::size_type,
                                     std::vector<int>::size_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::difference_type,
                                     std::vector<int>::difference_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::pointer,
                                     std::vector<int>::pointer>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::const_pointer,
                                     std::vector<int>::const_pointer>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::reference,
                                     std::vector<int>::reference>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::const_reference,
                                     std::vector<int>::const_reference>);
}

void test_vector_default_constructor(){
    static_assert(
        std::is_nothrow_default_constructible_v<c20_stl::vector<int>>
        == std::is_nothrow_default_constructible_v<std::vector<int>>
    );

    c20_stl::vector<int> value;
    std::vector<int> std_value;
}

void test_vector_empty(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.empty()),
                                     decltype(std_value.empty())>);
    static_assert(noexcept(value.empty()) == noexcept(std_value.empty()));

    assert(value.empty());
    assert(value.empty() == std_value.empty());
}

void test_vector_size(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.size()),
                                     decltype(std_value.size())>);
    static_assert(noexcept(value.size()) == noexcept(std_value.size()));

    assert(value.size() == 0);
    assert(value.size() == std_value.size());
    assert(value.empty() == (value.size() == 0));
}

void test_vector_capacity(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.capacity()),
                                     decltype(std_value.capacity())>);
    static_assert(noexcept(value.capacity()) == noexcept(std_value.capacity()));

    assert(value.capacity() == 0);
    assert(value.capacity() == std_value.capacity());
    assert(value.capacity() >= value.size());
}

}

int main() {
    c20_stl::test_vector_type();
    c20_stl::test_vector_default_constructor();
    c20_stl::test_vector_empty();
    c20_stl::test_vector_size();
    c20_stl::test_vector_capacity();

}
