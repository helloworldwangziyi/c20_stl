#ifndef C20_STL_VECTOR_HPP
#define C20_STL_VECTOR_HPP
#include <cstddef>
#include <c20_stl/memory.hpp>

namespace c20_stl{

template <typename T>
class vector{
public:

    using allocator_type = allocator<T>;

    using value_type = typename allocator_type::value_type;
    using size_type = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using pointer = typename allocator_type::pointer;
    using const_pointer = typename allocator_type::const_pointer;
    using reference = typename allocator_type::reference;
    using const_reference = typename allocator_type::const_reference;

    // 默认构造
    vector() noexcept : allocator_(), start_(nullptr) , finish_(nullptr), end_of_storage_(nullptr) {}

    // 析构
    ~vector() noexcept {
        pointer current_ = start_;
        while(current_ != finish_){
            destroy_at(current_);
            ++current_;
        }

        if(start_ != nullptr){
            allocator_.deallocate(start_, capacity());
        }
    }



    bool empty() const noexcept {
        return start_ == finish_;
    }

    size_type size() const noexcept {
        return static_cast<size_type>(finish_ - start_);
    }

    size_type capacity() const noexcept {
        return static_cast<size_type>(end_of_storage_ - start_);
    }



private:

    allocator_type allocator_;

    pointer start_;
    pointer finish_;
    pointer end_of_storage_;

};


}

#endif
