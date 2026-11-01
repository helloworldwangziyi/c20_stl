#ifndef C20_STL_TYPE_TRAITS_HPP
#define C20_STL_TYPE_TRAITS_HPP

#include <cstddef>

//  类型特征

namespace c20_stl{

template<typename T, T Value>
struct integral_constant{
    static constexpr T value = Value;

    using value_type = T;
    using type = integral_constant;

    constexpr operator value_type() const noexcept 
    {
        return value;
    }

    constexpr value_type operator()() const noexcept
    {
        return value;
    }
};

template<bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename T, typename U>
struct is_same : false_type{};


template <typename T>
struct is_same<T, T> : true_type{};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template <typename T>
struct is_void : false_type{};

template <>
struct is_void<void> : true_type{};

template <>
struct is_void<const void> : true_type{};

template <>
struct is_void<volatile void> : true_type{};

template <>
struct is_void<const volatile void> : true_type{};

template <typename T>
inline constexpr bool is_void_v = is_void<T>::value;

template <typename T>
struct is_null_pointer : false_type{};

template <>
struct is_null_pointer<decltype(nullptr)> : true_type{};

template <>
struct is_null_pointer<const decltype(nullptr)> : true_type{};

template <>
struct is_null_pointer<volatile decltype(nullptr)> : true_type{};

template <>
struct is_null_pointer<const volatile decltype(nullptr)> : true_type{};

template <typename T>
inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

template <typename T>
struct is_const : false_type{};

template <typename T>
struct is_const<const T> : true_type{};

template <typename T>
inline constexpr bool is_const_v = is_const<T>::value;

template <typename T>
struct is_volatile : false_type{};

template <typename T>
struct is_volatile<volatile T> : true_type{};

template <typename T>
inline constexpr bool is_volatile_v = is_volatile<T>::value;

template <typename T>
struct remove_const {using type = T;};

template <typename T>
struct remove_const <const T> {using type = T;};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

template <typename T>
struct add_const {using type = const T; };

template <typename T>
using add_const_t = typename add_const<T>::type;

template <typename T>
struct remove_volatile { using type = T;};

template <typename T>
struct remove_volatile<volatile T> {using type = T;};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T>
struct remove_reference {using type = T;};

template <typename T>
struct remove_reference<T&> {using type = T;};

template <typename T>
struct remove_reference<T&&> {using type = T;};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

template <typename T>
struct add_lvalue_reference {using type = T&;};

template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <typename T>
struct add_rvalue_reference {using type = T&&;};

template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

template <typename T>
struct is_array : false_type{};

template <typename T, std::size_t N>
struct is_array<T[N]> : true_type{};

template <typename T>
struct is_array<T[]> : true_type{};

template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

template <typename T>
struct rank : integral_constant<std::size_t, 0>{};

template <typename T>
struct rank<T[]> : integral_constant<std::size_t, 1 + rank<T>::value>{};

template <typename T, std::size_t N>
struct rank<T[N]> : integral_constant<std::size_t, 1 + rank<T>::value>{};

template <typename T>
inline constexpr std::size_t rank_v = rank<T>::value;

template <typename T, unsigned Dim = 0>
struct extent : integral_constant<std::size_t, 0>{};

template <typename T, unsigned Dim>
struct extent<T[],Dim> : integral_constant<std::size_t, (Dim == 0 ? 0 : extent<T, Dim - 1>::value)>{};

template <typename T, std::size_t N, unsigned Dim>
struct extent<T[N], Dim> : integral_constant<std::size_t, (Dim == 0 ? N : extent<T, Dim - 1>::value)>{};

template <typename T, unsigned Dim = 0>
inline constexpr std::size_t extent_v = extent<T, Dim>::value;

template <bool N, typename T, typename U>
struct conditional {
    using type = T;
};

template <typename T ,typename U>
struct conditional<false, T, U>{
    using type = U;
};

template <bool N, typename T, typename U>
using conditional_t = typename conditional<N, T ,U>::type;


// SFINAE 机制 替换失败不是错误
template <bool N, typename T = void>
struct enable_if{
};

template <typename T>
struct enable_if<true, T>{
    using type = T;
};

template <bool N, typename T = void>
using enable_if_t = typename enable_if<N, T>::type;

}


#endif