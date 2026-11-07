#ifndef C20_STL_MEMORY_HPP
#define C20_STL_MEMORY_HPP

#include <cstddef>
#include <new>
#include <utility>

namespace c20_stl{

template <typename T>
class allocator{
public:
    using value_type = T;

    allocator() noexcept = default;

    template <typename U>
    allocator(const allocator<U>&) noexcept {}

    T* allocate(std::size_t n){
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t) noexcept{
        ::operator delete(p);
    }
};

template <typename T, typename ...Args>
T* construct_at(T* p, Args&&... args){
    return new (p) T(std::forward<Args>(args)...);
}

template <typename T>
void destroy_at(T* p){
    p->~T();
}

template <typename ForwardIt, typename T>
void uninitialized_fill(ForwardIt first, ForwardIt last, const T& value){
    for (; first != last; ++first) {
        construct_at(&*first, value);
    }
}
template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_copy(InputIt first , InputIt last, ForwardIt dest)
{
    for (; first != last; ++first, ++dest)
    {
        construct_at(&*dest, *(first));
    }
    return dest;
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt dest)
{
    for (; first != last; ++first, ++dest)
    {
        construct_at(&*dest, std::move(*first));
    }
    return dest;
}

}

#endif
