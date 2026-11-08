#ifndef C20_STL_UTIL_HPP
#define C20_STL_UTIL_HPP

#include <c20_stl/type_traits.hpp>

namespace c20_stl{

template<typename T>
constexpr remove_reference_t<T>&& move(T&& value) noexcept
{
    return static_cast<remove_reference_t<T>&&>(value);
}

}

#endif
