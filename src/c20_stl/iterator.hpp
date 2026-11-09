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


template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
struct iterator{
    typedef Category iterator_category;
    typedef T value_type;
    typedef Distance difference_type;
    typedef Pointer pointer;
    typedef Reference reference;
};




}


#endif
