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

template <typename T1, typename T2>
struct pair{
    T1 first;
    T2 second;

    pair() : first(), second() {};

    template <typename U, typename V>
    pair(U&& a, V&& b) : first(c20_stl::forward<U>(a)), second(c20_stl::forward<V>(b)) {}

    template <typename U1, typename U2>
    pair(const pair<U1, U2>& other) : first(other.first), second(other.second) {}

    template <typename U1, typename U2>
    pair(pair<U1, U2>&& other) : first(c20_stl::forward<U1>(other.first)), second(c20_stl::forward<U2>(other.second)) {}

    

};

}

#endif
