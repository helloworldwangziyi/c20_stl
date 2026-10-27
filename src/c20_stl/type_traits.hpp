#ifndef C20_STL_TYPE_TRAITS_HPP
#define C20_STL_TYPE_TRAITS_HPP

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

}


#endif