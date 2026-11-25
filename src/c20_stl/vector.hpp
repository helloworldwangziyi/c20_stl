#ifndef C20_STL_VECTOR_HPP
#define C20_STL_VECTOR_HPP
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <limits>
#include <type_traits>
#include <c20_stl/memory.hpp>
#include <c20_stl/util.hpp>

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

    using iterator = pointer;
    using const_iterator = const_pointer;

    // 默认构造
    vector() noexcept : allocator_(), start_(nullptr) , finish_(nullptr), end_of_storage_(nullptr) {}

    vector(std::initializer_list<T> values)
    : allocator_(),
      start_(nullptr),
      finish_(nullptr),
      end_of_storage_(nullptr)
    {
      if (values.size() == 0) {
        return;
      }

      start_ = allocator_.allocate(values.size());
      finish_ = start_;
      end_of_storage_ = start_ + values.size();

      try{
        for(const auto& value : values){
            construct_at(finish_, value);
            ++finish_;
        }
      }catch(...){
        while(finish_ != start_){
            --finish_;
            destroy_at(finish_);

        }
        allocator_.deallocate(start_, values.size());
        start_ = nullptr;
        finish_ = nullptr;
        end_of_storage_ = nullptr;

        throw;
      }

    }

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

    // 移动构造
    vector(vector&& other) noexcept : vector() {
        swap(other);
    }

    // 拷贝构造
    vector(const vector& other) : vector() {
        if(other.empty()){
            return;
        }

        this->start_ = allocator_.allocate(other.size());
        this->finish_ = start_;
        this->end_of_storage_ = start_ + other.size();

        try{
            for(const auto& val : other){
                construct_at(finish_, val);
                ++finish_;
            }
        }
        catch(...){
            while(finish_ != start_){
                --finish_;
                destroy_at(finish_);
            }
            allocator_.deallocate(start_, other.size());

            start_ = nullptr;
            finish_ = nullptr;
            end_of_storage_ = nullptr;
            throw;
        }
    }

    vector& operator=(const vector& other){
        vector tmp(other);
        swap(tmp);
        return *this;
    }

    vector& operator=(vector&& other) noexcept{
        vector tmp(c20_stl::move(other));
        swap(tmp);
        return *this;
    }

    void swap(vector& other) noexcept{
        c20_stl::swap(start_, other.start_);
        c20_stl::swap(finish_, other.finish_);
        c20_stl::swap(end_of_storage_ ,other.end_of_storage_);
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
        return start_ == nullptr
             ? 0
             : static_cast<size_type>(finish_ - start_);
    }

    size_type capacity() const noexcept {
        return start_ == nullptr
             ? 0
             : static_cast<size_type>(end_of_storage_ - start_);
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

    reference at(size_type pos){
        if (pos >= size()) {
            throw std::out_of_range("vector::at: index out of range");
        }
        return this->start_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("vector::at: index out of range");
        }
        return this->start_[pos];
    }

    iterator begin() noexcept {
        return start_;
    }

    const_iterator begin() const noexcept{
        return start_;
    }

    iterator end() noexcept {
        return finish_;
    }

    const_iterator end() const noexcept{
        return finish_;
    }

    const_iterator cbegin() const noexcept{
        return start_;
    }

    const_iterator cend() const noexcept{
        return finish_;
    }

    size_type max_size() const noexcept{
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    void reserve(size_type new_cap){
        if(new_cap > max_size()){
            throw std::length_error("vector::reserve: capacity too large");
        }
        if(new_cap <= capacity()){
            return;
        }
        if(new_cap > capacity()){
            // 保存旧状态
            const size_type old_size = size();
            const size_type old_capacity = capacity();
            pointer old_start_ = start_;
            pointer old_finish_ = finish_;

            auto new_start_ = allocator_.allocate(new_cap);
            auto new_finish_ = new_start_;


            try{
                for(size_type i = 0; i < old_size; ++i){
                    // 判断一下是否支持移动构造
                    if constexpr(std::is_nothrow_move_constructible_v<T>||!std::is_copy_constructible_v<T>){
                        construct_at(new_finish_, c20_stl::move(old_start_[i]));
                    }
                    else{
                        construct_at(new_finish_, old_start_[i]);
                    }
                    ++new_finish_;
                }
            }
            catch(...){
                while(new_finish_ != new_start_){
                    --new_finish_;
                    destroy_at(new_finish_);
                }
                allocator_.deallocate(new_start_, new_cap);
                throw;
            }

            while(old_finish_ != old_start_){
                --old_finish_;
                destroy_at(old_finish_);
            }

            if(old_start_ != nullptr){
                allocator_.deallocate(old_start_, old_capacity);
            }

            start_ = new_start_;
            finish_ = new_finish_;
            end_of_storage_ = new_start_ + new_cap;
        }
    }

    void push_back(const T& value){
        if(size() == capacity()){
            if(size() == max_size()){
                throw std::length_error("vector::push_back: size reached max_size");
            }
            reserve(recommend_capacity(size() + 1));
        }
        construct_at(finish_, value);
        ++finish_;
    }

    void push_back(T&& value){
        if(size() == capacity()){
            if(size() == max_size()){
                throw std::length_error("vector::push_back: size reached max_size");
            }
            reserve(recommend_capacity(size() + 1));
        }
        construct_at(finish_, std::move(value));
        ++finish_;
    }

    void pop_back(){
        --finish_;
        destroy_at(finish_);
    }

    void clear() noexcept{
        if(empty()) {
            return;
        }
        while(finish_ != start_){
            --finish_;
            destroy_at(finish_);
        }
    }

private:
    size_type recommend_capacity(size_type requested) const{
        const size_type ms = max_size();
        if(requested > ms){
            throw std::length_error("vector::recommend_capacity: capacity too large");
        }
        const size_type cap = capacity();
        if(cap >= ms / 2) return ms;
        return (cap == 0) ? 1 : cap * 2;
    }

    allocator_type allocator_;
    pointer start_;
    pointer finish_;
    pointer end_of_storage_;

};


}

#endif
