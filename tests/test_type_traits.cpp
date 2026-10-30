#include <iostream>
#include <type_traits>
#include <c20_stl/type_traits.hpp>


namespace c20_stl{


void test_traits(){
    // 嵌套类型要用 :: 访问(此处非模板上下文,不需要 typename)
    static_assert(std::is_same_v<c20_stl::true_type::type, c20_stl::true_type>);
    static_assert(c20_stl::true_type{});
    static_assert(!c20_stl::false_type{});

    // test is_same_v 
    static_assert(c20_stl::is_same_v<c20_stl::true_type::type, c20_stl::true_type>);
    static_assert(c20_stl::is_same_v<int, int>);
    static_assert(!c20_stl::is_same_v<int, bool>);
    static_assert(c20_stl::is_same_v<int,int> == std::is_same_v<int, int>);

    // is_void:四种 cv 组合 + 对照 std + 承重
    static_assert(!c20_stl::is_void_v<int>);
    static_assert(c20_stl::is_void_v<void>);
    static_assert(c20_stl::is_void_v<const void>);
    static_assert(c20_stl::is_void_v<volatile void>);
    static_assert(c20_stl::is_void_v<const volatile void>);
    static_assert(c20_stl::is_void_v<int> ==
                  std::is_void_v<int>);
    static_assert(c20_stl::is_void_v<void> ==
                  std::is_void_v<void>);
    static_assert(std::is_base_of_v<c20_stl::false_type,
                                    c20_stl::is_void<int>>);

    // is_null_pointer:同样配置
    static_assert(c20_stl::is_null_pointer_v<decltype(nullptr)>);
    static_assert(!c20_stl::is_null_pointer_v<int>);
    static_assert(!c20_stl::is_null_pointer_v<void>);
    static_assert(c20_stl::is_null_pointer_v<const decltype(nullptr)>);
    static_assert(c20_stl::is_null_pointer_v<decltype(nullptr)> ==
                  std::is_null_pointer_v<decltype(nullptr)>);
    static_assert(std::is_base_of_v<c20_stl::false_type,
                                    c20_stl::is_null_pointer<int>>);
}

}

int main(){
    using namespace c20_stl;
    test_traits(); 
}
