#ifndef C20_STL_MEMORY_HPP
#define C20_STL_MEMORY_HPP

#include <cstddef>
#include <new>

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

}

#endif
