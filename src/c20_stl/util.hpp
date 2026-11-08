#ifndef C20_STL_UTIL_HPP
#define C20_STL_UTIL_HPP

#include <c20_stl/type_traits.hpp>

namespace c20_stl{

template<typename T>
constexpr remove_reference_t<T>&& move(T&& value) noexcept
{
    return static_cast<remove_reference_t<T>&&>(value);
}

template<typename T>
constexpr T&& forward(remove_reference_t<T>& value) noexcept
{
    return static_cast<T&&>(value);
}

template<typename T>
constexpr T&& forward(remove_reference_t<T>&& value) noexcept
{
    static_assert(!is_lvalue_reference_v<T>, "bad forward call");
    return static_cast<T&&>(value);
}

template <typename T>
void swap(T& a, T& b){
    T tmp = move(a);
    a = move(b);
    b = move(tmp);
}

}

#endif
