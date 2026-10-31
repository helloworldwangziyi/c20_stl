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

    // is_const:重点是顶层 const 与底层 const 的分界
    static_assert(c20_stl::is_const_v<const int>);
    static_assert(!c20_stl::is_const_v<int>);
    static_assert(c20_stl::is_const_v<int *const>); // 顶层:const 管指针自己
    static_assert(!c20_stl::is_const_v<const int *>); // 底层:const 管指向的对象, 不算 
    static_assert(c20_stl::is_const_v<int *const> == std::is_const_v<int *const>);
    static_assert(std::is_base_of_v<c20_stl::false_type,
                                    c20_stl::is_const<int>>);

    // is_volatile
    static_assert(c20_stl::is_volatile_v<volatile int>);
    static_assert(!c20_stl::is_volatile_v<int>);
    static_assert(c20_stl::is_volatile_v<int* volatile>);
    static_assert(!c20_stl::is_volatile_v<volatile int*>);

    static_assert(c20_stl::is_volatile_v<int* volatile> == std::is_volatile_v<int* volatile>);
    static_assert(std::is_base_of_v<c20_stl::false_type, c20_stl::is_volatile<int>>);


    // test remove_const
    static_assert(c20_stl::is_same_v<c20_stl::remove_const_t<const int>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_const_t<const int*>, const int*>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_const_t<int>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_const_t<const volatile int>, volatile int>);  // 只剥 const,留 volatile
    static_assert(c20_stl::is_same_v<c20_stl::remove_const_t<const int*>, std::remove_const_t<const int*>>);

    // test add_const

    static_assert(c20_stl::is_same_v<c20_stl::add_const_t<int>, const int>);
    static_assert(c20_stl::is_same_v<c20_stl::add_const_t<int>, std::add_const_t<int>>);
    static_assert(c20_stl::is_same_v<c20_stl::add_const_t<const int>, const int>);
    static_assert(c20_stl::is_same_v<c20_stl::add_const_t<volatile int>, const volatile int>);  // 加在已有 volatile 的顶层

    // test remove_volatile
    static_assert(c20_stl::is_same_v<c20_stl::remove_volatile_t<volatile int>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_volatile_t<volatile int*>, volatile int*>);  // 灵魂:底层 volatile 不动
    static_assert(c20_stl::is_same_v<c20_stl::remove_volatile_t<int>, int>);                      // 原样放行(主模板分支)
    static_assert(c20_stl::is_same_v<c20_stl::remove_volatile_t<const volatile int>, const int>); // 只剥 volatile,留 const
    static_assert(c20_stl::is_same_v<c20_stl::remove_volatile_t<volatile int*>, std::remove_volatile_t<volatile int*>>);

    // test remove_reference
    static_assert(c20_stl::is_same_v<c20_stl::remove_reference_t<int&>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_reference_t<int&&>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::remove_reference_t<int>, int>);                    // 原样放行(主模板分支)
    static_assert(c20_stl::is_same_v<c20_stl::remove_reference_t<int*&>, int*>);                 // 灵魂:指针不是引用
    static_assert(c20_stl::is_same_v<c20_stl::remove_reference_t<int&&>, std::remove_reference_t<int&&>>);

    // test add_lvalue_reference:单主模板,引用折叠兜底
    static_assert(c20_stl::is_same_v<c20_stl::add_lvalue_reference_t<int>, int&>);
    static_assert(c20_stl::is_same_v<c20_stl::add_lvalue_reference_t<int&>, int&>);
    static_assert(c20_stl::is_same_v<c20_stl::add_lvalue_reference_t<int&&>, int&>);   // 灵魂:右被左吸
    // test add_rvalue_reference
    static_assert(c20_stl::is_same_v<c20_stl::add_rvalue_reference_t<int>, int&&>);
    static_assert(c20_stl::is_same_v<c20_stl::add_rvalue_reference_t<int&&>, int&&>);
    static_assert(c20_stl::is_same_v<c20_stl::add_rvalue_reference_t<int&>, int&>);    // 灵魂:左值赢
    // std 抽查
    static_assert(c20_stl::is_same_v<c20_stl::add_lvalue_reference_t<int&&>, std::add_lvalue_reference_t<int&&>>);
    static_assert(c20_stl::is_same_v<c20_stl::add_rvalue_reference_t<int&>, std::add_rvalue_reference_t<int&>>);
}

}

int main(){
    using namespace c20_stl;
    test_traits(); 
}
