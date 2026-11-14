#ifndef C20_STL_ITERATOR_HPP
#define C20_STL_ITERATOR_HPP

#include <cstddef>
namespace c20_stl{

// 五种迭代器的类型
struct input_iterator_tag{};

struct output_iterator_tag{};

struct forward_iterator_tag : input_iterator_tag{};

struct bidirectional_iterator_tag : forward_iterator_tag{};

struct random_access_iterator_tag : bidirectional_iterator_tag{};

struct contiguous_iterator_tag : random_access_iterator_tag{};


template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
struct iterator{
    typedef Category iterator_category;
    typedef T value_type;
    typedef Distance difference_type;
    typedef Pointer pointer;
    typedef Reference reference;
};

template <typename Iterator>
struct iterator_traits{
    using iterator_category = typename Iterator::iterator_category;
    using value_type = typename Iterator::value_type;
    using difference_type = typename Iterator::difference_type;
    using pointer = typename Iterator::pointer;
    using reference = typename Iterator::reference;
};

template <typename T>
struct iterator_traits<T*>{
    using iterator_category = random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
};

template <typename T>
struct iterator_traits<const T*>{
    using iterator_category = random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
};


template <typename Iterator>
iterator_traits<Iterator>::difference_type distance_impl(Iterator first, Iterator last, input_iterator_tag){
    typename iterator_traits<Iterator>::difference_type n = 0;
    for(; first != last; ++first, ++n);
    return n;
}

template <typename Iterator>
iterator_traits<Iterator>::difference_type distance_impl(Iterator first, Iterator last, random_access_iterator_tag){
    return last - first;
}

template <typename Iterator>
iterator_traits<Iterator>::difference_type distance(Iterator first, Iterator last){
    return distance_impl(first, last, typename iterator_traits<Iterator>::iterator_category{});
}



}


#endif
