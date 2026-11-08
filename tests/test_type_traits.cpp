#include <iostream>
#include <type_traits>
#include <c20_stl/type_traits.hpp>


namespace c20_stl{

template <typename T>
constexpr c20_stl::enable_if_t<c20_stl::is_same_v<T, int>, int>
test_enable_if_select(T) {
    return 1;
}

template <typename T>
constexpr c20_stl::enable_if_t<!c20_stl::is_same_v<T, int>, int>
test_enable_if_select(T) {
    return 2;
}

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

    // test is_array
    static_assert(c20_stl::is_array_v<int[]>);
    static_assert(c20_stl::is_array_v<int[3]>);
    static_assert(c20_stl::is_array_v<int[2][3]>);       // 多维数组也算数组
    static_assert(!c20_stl::is_array_v<int>);
    static_assert(!c20_stl::is_array_v<int*>);
    static_assert(!c20_stl::is_array_v<int(*)[3]>);      // 灵魂:指向数组的指针仍不是数组

    // std 裁判:未知长度数组、多维数组、指针各抽查一次
    static_assert(c20_stl::is_array_v<int[]> == std::is_array_v<int[]>);
    static_assert(c20_stl::is_array_v<int[2][3]> == std::is_array_v<int[2][3]>);
    static_assert(c20_stl::is_array_v<int*> == std::is_array_v<int*>);

    // test rank
    static_assert(c20_stl::rank_v<int[]> == 1);
    static_assert(c20_stl::rank_v<int> == 0);
    static_assert(c20_stl::rank_v<int*> == 0);
    static_assert(c20_stl::rank_v<int[3]> == 1);
    static_assert(c20_stl::rank_v<int[2][3]> == 2);
    static_assert(c20_stl::rank_v<int[2][3][4]> == 3);
    static_assert(c20_stl::rank_v<int*[3]> == 1);
    static_assert(c20_stl::rank_v<int(*)[3]> == 0);

    // std 裁判:未知长度数组、多维数组、数组指针各抽查一次
    static_assert(c20_stl::rank_v<int[]> == std::rank_v<int[]>);
    static_assert(c20_stl::rank_v<int[2][3][4]> == std::rank_v<int[2][3][4]>);
    static_assert(c20_stl::rank_v<int(*)[3]> == std::rank_v<int(*)[3]>);

    // test extent
    static_assert(c20_stl::extent<int[3]>::value == 3);  // 类模板默认查询第 0 维
    static_assert(c20_stl::extent_v<int> == 0);
    static_assert(c20_stl::extent_v<int*> == 0);
    static_assert(c20_stl::extent_v<int[3]> == 3);
    static_assert(c20_stl::extent_v<int[3], 1> == 0);     // 维度越界返回 0
    static_assert(c20_stl::extent_v<int[]> == 0);         // 当前维长度未知
    static_assert(c20_stl::extent_v<int[2][3]> == 2);
    static_assert(c20_stl::extent_v<int[2][3], 1> == 3);
    static_assert(c20_stl::extent_v<int[2][3], 2> == 0);
    static_assert(c20_stl::extent_v<int[][3]> == 0);
    static_assert(c20_stl::extent_v<int[][3], 1> == 3);   // 外层未知,内层仍可查询
    static_assert(c20_stl::extent_v<int*[3]> == 3);
    static_assert(c20_stl::extent_v<int(*)[3]> == 0);

    // std 裁判:定长、多维、未知长度数组、数组指针各抽查一次
    static_assert(c20_stl::extent_v<int[3]> == std::extent_v<int[3]>);
    static_assert(c20_stl::extent_v<int[2][3], 1> == std::extent_v<int[2][3], 1>);
    static_assert(c20_stl::extent_v<int[][3], 1> == std::extent_v<int[][3], 1>);
    static_assert(c20_stl::extent_v<int(*)[3]> == std::extent_v<int(*)[3]>);

    // test conditional
    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<true, int, bool>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<false, int, bool>, bool>);

    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<true, void, int>, void>);
    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<true, int*, int>, int *>);

    // std 裁判:两个选择方向各对照一次
    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<true, int, bool>,
                                     std::conditional_t<true, int, bool>>);
    static_assert(c20_stl::is_same_v<c20_stl::conditional_t<false, int, bool>,
                                     std::conditional_t<false, int, bool>>);

    // test enable_if
    static_assert(c20_stl::is_same_v<c20_stl::enable_if_t<true>, void>);
    static_assert(c20_stl::is_same_v<c20_stl::enable_if_t<true, int>, int>);
    static_assert(c20_stl::is_same_v<c20_stl::enable_if_t<true, int>,
                                     std::enable_if_t<true, int>>);

    // SFINAE:int 候选与非 int 候选各退出一次,留下的函数给出不同答案
    static_assert(test_enable_if_select(42) == 1);
    static_assert(test_enable_if_select(3.14) == 2);

    // test is_integral:覆盖 C++20 标准整数类型
    static_assert(c20_stl::is_integral_v<bool>);
    static_assert(c20_stl::is_integral_v<char>);
    static_assert(c20_stl::is_integral_v<signed char>);
    static_assert(c20_stl::is_integral_v<unsigned char>);
    static_assert(c20_stl::is_integral_v<wchar_t>);
    static_assert(c20_stl::is_integral_v<char8_t>);
    static_assert(c20_stl::is_integral_v<char16_t>);
    static_assert(c20_stl::is_integral_v<char32_t>);
    static_assert(c20_stl::is_integral_v<short>);
    static_assert(c20_stl::is_integral_v<unsigned short>);
    static_assert(c20_stl::is_integral_v<int>);
    static_assert(c20_stl::is_integral_v<unsigned int>);
    static_assert(c20_stl::is_integral_v<long>);
    static_assert(c20_stl::is_integral_v<unsigned long>);
    static_assert(c20_stl::is_integral_v<long long>);
    static_assert(c20_stl::is_integral_v<unsigned long long>);

    // cv 限定不改变整数身份
    static_assert(c20_stl::is_integral_v<const int>);
    static_assert(c20_stl::is_integral_v<volatile int>);
    static_assert(c20_stl::is_integral_v<const volatile int>);

    // 浮点数、指针、引用、void 都不是整数类型
    static_assert(!c20_stl::is_integral_v<float>);
    static_assert(!c20_stl::is_integral_v<double>);
    static_assert(!c20_stl::is_integral_v<int*>);
    static_assert(!c20_stl::is_integral_v<int&>);
    static_assert(!c20_stl::is_integral_v<void>);

    // std 裁判:字符类型、cv、负例、引用各抽查一次
    static_assert(c20_stl::is_integral_v<char8_t> == std::is_integral_v<char8_t>);
    static_assert(c20_stl::is_integral_v<const volatile long long> ==
                  std::is_integral_v<const volatile long long>);
    static_assert(c20_stl::is_integral_v<double> == std::is_integral_v<double>);
    static_assert(c20_stl::is_integral_v<int&> == std::is_integral_v<int&>);

    // test is_floating_point
    static_assert(c20_stl::is_floating_point_v<float>);
    static_assert(c20_stl::is_floating_point_v<double>);
    static_assert(c20_stl::is_floating_point_v<long double>);
    static_assert(c20_stl::is_floating_point_v<const float>);
    static_assert(c20_stl::is_floating_point_v<volatile double>);
    static_assert(c20_stl::is_floating_point_v<const volatile long double>);
    static_assert(!c20_stl::is_floating_point_v<int>);
    static_assert(!c20_stl::is_floating_point_v<float&>);
    static_assert(!c20_stl::is_floating_point_v<double*>);
    static_assert(!c20_stl::is_floating_point_v<void>);
    static_assert(c20_stl::is_floating_point_v<const volatile long double> ==
                  std::is_floating_point_v<const volatile long double>);
    static_assert(c20_stl::is_floating_point_v<int> == std::is_floating_point_v<int>);

    // test is_arithmetic:整数或浮点数都属于算术类型
    static_assert(c20_stl::is_arithmetic_v<int>);
    static_assert(c20_stl::is_arithmetic_v<unsigned long long>);
    static_assert(c20_stl::is_arithmetic_v<float>);
    static_assert(c20_stl::is_arithmetic_v<const double>);
    static_assert(c20_stl::is_arithmetic_v<volatile long double>);
    static_assert(!c20_stl::is_arithmetic_v<void>);
    static_assert(!c20_stl::is_arithmetic_v<int*>);
    static_assert(!c20_stl::is_arithmetic_v<int&>);
    static_assert(!c20_stl::is_arithmetic_v<c20_stl::true_type>);
    static_assert(c20_stl::is_arithmetic_v<const volatile long double> ==
                  std::is_arithmetic_v<const volatile long double>);
    static_assert(c20_stl::is_arithmetic_v<int*> == std::is_arithmetic_v<int*>);

    // test is_signed:有符号整数和浮点数为 true
    static_assert(c20_stl::is_signed_v<signed char>);
    static_assert(c20_stl::is_signed_v<short>);
    static_assert(c20_stl::is_signed_v<int>);
    static_assert(c20_stl::is_signed_v<long>);
    static_assert(c20_stl::is_signed_v<long long>);
    static_assert(c20_stl::is_signed_v<float>);
    static_assert(c20_stl::is_signed_v<double>);
    static_assert(c20_stl::is_signed_v<long double>);
    static_assert(c20_stl::is_signed_v<const int>);
    static_assert(c20_stl::is_signed_v<const volatile double>);

    static_assert(!c20_stl::is_signed_v<bool>);
    static_assert(!c20_stl::is_signed_v<unsigned char>);
    static_assert(!c20_stl::is_signed_v<unsigned short>);
    static_assert(!c20_stl::is_signed_v<unsigned int>);
    static_assert(!c20_stl::is_signed_v<unsigned long>);
    static_assert(!c20_stl::is_signed_v<unsigned long long>);
    static_assert(!c20_stl::is_signed_v<char8_t>);
    static_assert(!c20_stl::is_signed_v<char16_t>);
    static_assert(!c20_stl::is_signed_v<char32_t>);
    static_assert(!c20_stl::is_signed_v<void>);
    static_assert(!c20_stl::is_signed_v<int*>);
    static_assert(!c20_stl::is_signed_v<int&>);
    static_assert(!c20_stl::is_signed_v<c20_stl::true_type>);

    // char/wchar_t 的符号由平台决定,必须让 std 当裁判
    static_assert(c20_stl::is_signed_v<char> == std::is_signed_v<char>);
    static_assert(c20_stl::is_signed_v<wchar_t> == std::is_signed_v<wchar_t>);
    static_assert(c20_stl::is_signed_v<unsigned int> == std::is_signed_v<unsigned int>);
    static_assert(c20_stl::is_signed_v<const volatile double> ==
                  std::is_signed_v<const volatile double>);
    static_assert(c20_stl::is_signed_v<c20_stl::true_type> ==
                  std::is_signed_v<c20_stl::true_type>);

    // test is_lvalue_reference:只有 T& 形式为 true
    static_assert(!c20_stl::is_lvalue_reference_v<int>);
    static_assert(c20_stl::is_lvalue_reference_v<int&>);
    static_assert(!c20_stl::is_lvalue_reference_v<int&&>);
    static_assert(c20_stl::is_lvalue_reference_v<const int&>);
    static_assert(c20_stl::is_lvalue_reference_v<int> == std::is_lvalue_reference_v<int>);
    static_assert(c20_stl::is_lvalue_reference_v<int&> == std::is_lvalue_reference_v<int&>);
    static_assert(c20_stl::is_lvalue_reference_v<int&&> == std::is_lvalue_reference_v<int&&>);
    static_assert(c20_stl::is_lvalue_reference_v<const int&> ==
                  std::is_lvalue_reference_v<const int&>);

    // test is_rvalue_reference:只有 T&& 形式为 true
    static_assert(!c20_stl::is_rvalue_reference_v<int>);
    static_assert(!c20_stl::is_rvalue_reference_v<int&>);
    static_assert(c20_stl::is_rvalue_reference_v<int&&>);
    static_assert(!c20_stl::is_rvalue_reference_v<const int&>);
    static_assert(c20_stl::is_rvalue_reference_v<int> == std::is_rvalue_reference_v<int>);
    static_assert(c20_stl::is_rvalue_reference_v<int&> == std::is_rvalue_reference_v<int&>);
    static_assert(c20_stl::is_rvalue_reference_v<int&&> == std::is_rvalue_reference_v<int&&>);
    static_assert(c20_stl::is_rvalue_reference_v<const int&&> ==
                  std::is_rvalue_reference_v<const int&&>);

}

}

int main(){
    using namespace c20_stl;
    test_traits(); 
}
