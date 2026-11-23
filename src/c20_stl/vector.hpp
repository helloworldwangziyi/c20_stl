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

    // 填充构造
    vector(size_type count, const T& other)
    : allocator_(),
      start_(nullptr),
      finish_(nullptr),
      end_of_storage_(nullptr)
    {
        if(count == 0) {
            return;
        }
        // 分配内存
        start_ = allocator_.allocate(count);
        end_of_storage_ = start_ + count;
        finish_ = start_;

        try{
            for(size_type i = 0; i < count; ++i){
                construct_at(finish_, other);
                ++finish_;
            }
        } catch(...){
            while(finish_ != start_){
                --finish_;
                destroy_at(finish_);
            }
            allocator_.deallocate(start_, count);

            start_ = nullptr;
            finish_ = nullptr;
            end_of_storage_ = nullptr;
            throw;
        }

    }

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

    pointer data() noexcept{
        return start_;
    }

    const_pointer data() const noexcept{
        return start_;
    }

    reference operator[](size_type pos) noexcept {
        return *(start_ + pos);
    }

    const_reference operator[](size_type pos) const noexcept{
        return *(start_ + pos);
    }

    reference front() noexcept{
        return *start_;
    }

    const_reference front() const noexcept {
        return *start_;
    }

    reference back() noexcept{
        return *(finish_ - 1);
    }

    const_reference back() const noexcept {
        return *(finish_ - 1);
    }

private:
    allocator_type allocator_;
    pointer start_;
    pointer finish_;
    pointer end_of_storage_;

};


}

#endif
