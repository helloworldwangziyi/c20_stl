#ifndef C20_STL_TYPE_TRAITS_HPP
#define C20_STL_TYPE_TRAITS_HPP

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
constexpr inline bool is_same_v = is_same<T, U>::value;



}


#endif