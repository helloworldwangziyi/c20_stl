#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <c20_stl/vector.hpp>
#include <c20_stl/type_traits.hpp>
#include <type_traits>
#include <vector>

namespace c20_stl{

class Item {
public:
    int v;
    inline static int constructions = 0;
    inline static int destructions = 0;

    Item(int val) : v(val){
        ++constructions;
    }

    Item(const Item& other) : v(other.v){
        ++constructions;
    }

    ~Item(){
        ++destructions;
    }
};

class ThrowingItem{
public:
    inline static int alive = 0;
    inline static int copies_before_throw = 0;

    ThrowingItem(){++alive;}

    ThrowingItem(const ThrowingItem&){
        if(copies_before_throw == 0){
            throw 42;
        }
        --copies_before_throw;
        ++alive;
    }

    ~ThrowingItem(){
        --alive;
    }
};

void test_vector_type(){
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::value_type, int>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::allocator_type, c20_stl::allocator<int>>);
    static_assert(std::is_same_v<c20_stl::vector<int>::value_type,
                                 std::vector<int>::value_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::size_type,
                                     std::vector<int>::size_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::difference_type,
                                     std::vector<int>::difference_type>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::pointer,
                                     std::vector<int>::pointer>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::const_pointer,
                                     std::vector<int>::const_pointer>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::reference,
                                     std::vector<int>::reference>);
    static_assert(c20_stl::is_same_v<c20_stl::vector<int>::const_reference,
                                     std::vector<int>::const_reference>);
}

void test_vector_default_constructor(){
    static_assert(
        std::is_nothrow_default_constructible_v<c20_stl::vector<int>>
        == std::is_nothrow_default_constructible_v<std::vector<int>>
    );

    c20_stl::vector<int> value;
    std::vector<int> std_value;
}

void test_vector_empty(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.empty()),
                                     decltype(std_value.empty())>);
    static_assert(noexcept(value.empty()) == noexcept(std_value.empty()));

    assert(value.empty());
    assert(value.empty() == std_value.empty());
}

void test_vector_size(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.size()),
                                     decltype(std_value.size())>);
    static_assert(noexcept(value.size()) == noexcept(std_value.size()));

    assert(value.size() == 0);
    assert(value.size() == std_value.size());
    assert(value.empty() == (value.size() == 0));
}

void test_vector_capacity(){
    const c20_stl::vector<int> value;
    const std::vector<int> std_value;

    static_assert(c20_stl::is_same_v<decltype(value.capacity()),
                                     decltype(std_value.capacity())>);
    static_assert(noexcept(value.capacity()) == noexcept(std_value.capacity()));

    assert(value.capacity() == 0);
    assert(value.capacity() == std_value.capacity());
    assert(value.capacity() >= value.size());
}

void test_vector_destroy(){
    static_assert(
        std::is_nothrow_destructible_v<c20_stl::vector<int>>
        == std::is_nothrow_destructible_v<std::vector<int>>
    );

    Item::constructions = 0;
    Item::destructions = 0;

    {
        c20_stl::vector<Item> value;
    }
    assert(Item::constructions == 0);
    assert(Item::destructions == 0);
}

void test_vector_fill_constructor(){
    const c20_stl::vector<int> value(4, 7);
    const std::vector<int> std_value(4, 7);

    assert(value.empty() == std_value.empty());
    assert(value.size() == std_value.size());
    assert(value.capacity() == 4);

    const c20_stl::vector<int> empty_value(0, 7);
    const std::vector<int> std_empty_value(0, 7);

    assert(empty_value.empty() == std_empty_value.empty());
    assert(empty_value.size() == std_empty_value.size());
    assert(empty_value.capacity() == 0);
}

void test_vector_fill_lifecycle(){
    Item other(42);

    Item::constructions = 0;
    Item::destructions = 0;

    {
        c20_stl::vector<Item> value(3, other);

        assert(Item::constructions == 3);
        assert(Item::destructions == 0);
    }

    const int c20_constructions = Item::constructions;
    const int c20_destructions = Item::destructions;

    Item::constructions = 0;
    Item::destructions = 0;

    {
        std::vector<Item> std_value(3, other);
    }

    assert(c20_constructions == Item::constructions);
    assert(c20_destructions == Item::destructions);
    assert(c20_constructions == 3);
    assert(c20_destructions == 3);
}

void test_vector_fill_exception(){
    ThrowingItem::alive = 0;
    {
        ThrowingItem other;

        ThrowingItem::copies_before_throw = 2;
        bool c20_threw = false;

        try{
            c20_stl::vector<ThrowingItem> value(5, other);
        }
        catch(int){
            c20_threw = true;
        }
        assert(c20_threw);
        assert(ThrowingItem::alive == 1);

        ThrowingItem::copies_before_throw = 2;
        bool std_threw = false;

        try {
            std::vector<ThrowingItem> value(5, other);
        } catch(int) {
            std_threw = true;
        }

        assert(std_threw == c20_threw);
        assert(ThrowingItem::alive == 1);
    }
    assert(ThrowingItem::alive == 0);
}

void test_vector_initializer_list_constructor(){
    static_assert(std::is_constructible_v<c20_stl::vector<int>, std::initializer_list<int>> ==
                  std::is_constructible_v<std::vector<int>, std::initializer_list<int>>);

    c20_stl::vector<int> value{10, 20, 30, 40};
    std::vector<int> std_value{10, 20, 30, 40};

    assert(value.size() == std_value.size());
    assert(value.capacity() == value.size());

    for(c20_stl::vector<int>::size_type i = 0; i < value.size(); ++i){
        assert(value[i] == std_value[i]);
    }

    const std::initializer_list<int> empty_values{};
    c20_stl::vector<int> empty_value(empty_values);
    std::vector<int> std_empty_value(empty_values);

    assert(empty_value.empty());
    assert(empty_value.size() == std_empty_value.size());
    assert(empty_value.capacity() == 0);
    assert(empty_value.data() == nullptr);
}

void test_vector_initializer_list_exception(){
    ThrowingItem::alive = 0;
    {
        ThrowingItem other;
        ThrowingItem::copies_before_throw = 10;
        auto values = {other, other, other, other, other};
        assert(ThrowingItem::alive == 6);

        ThrowingItem::copies_before_throw = 2;
        bool c20_threw = false;
        try{
            c20_stl::vector<ThrowingItem> value(values);
        }
        catch(int){
            c20_threw = true;
        }

        assert(c20_threw);
        assert(ThrowingItem::alive == 6);

        ThrowingItem::copies_before_throw = 2;
        bool std_threw = false;
        try{
            std::vector<ThrowingItem> value(values);
        }
        catch(int){
            std_threw = true;
        }

        assert(std_threw);
        assert(ThrowingItem::alive == 6);
    }
    assert(ThrowingItem::alive == 0);
}

void test_vector_data(){
    c20_stl::vector<int> value(4, 7);
    std::vector<int> std_value(4, 7);

    static_assert(c20_stl::is_same_v<
        decltype(value.data()),
        decltype(std_value.data())>);

    static_assert(noexcept(value.data()) ==
        noexcept(std_value.data()));

    const auto &const_value = value;
    const auto &std_const_value = std_value;

    static_assert(c20_stl::is_same_v<decltype(const_value.data()),
                                     decltype(std_const_value.data())>);
    static_assert(noexcept(const_value.data()) ==
                  noexcept(std_const_value.data()));

    for (c20_stl::vector<int>::size_type i = 0; i < value.size(); ++i) {
        assert(value.data()[i] == std_value.data()[i]);
    }

    value.data()[1] = 42;
    std_value.data()[1] = 42;

    assert(value.data()[1] == std_value.data()[1]);
    assert(const_value.data()[1] == std_const_value.data()[1]);

    c20_stl::vector<int> empty_value;
    const c20_stl::vector<int> zero_value(0, 7);

    assert(empty_value.data() == nullptr);
    assert(zero_value.data() == nullptr);
}

void test_vector_subscript(){
    c20_stl::vector<int> value(3, 7);
    std::vector<int> std_value(3, 7);

    static_assert(c20_stl::is_same_v<decltype(value[0]),
                                     decltype(std_value[0])>);
    static_assert(noexcept(value[0]) == noexcept(std_value[0]));

    const auto& const_value = value;
    const auto& std_const_value = std_value;

    static_assert(c20_stl::is_same_v<decltype(const_value[0]),
                                     decltype(std_const_value[0])>);
    static_assert(noexcept(const_value[0]) == noexcept(std_const_value[0]));

    for(c20_stl::vector<int>::size_type i = 0; i < value.size(); ++i){
        assert(value[i] == std_value[i]);
    }

    value[1] = 42;
    std_value[1] = 42;

    assert(value[1] == std_value[1]);
    assert(const_value[1] == std_const_value[1]);

    auto& ref = value[0];
    auto& std_ref = std_value[0];

    ref = 9;
    std_ref = 9;

    assert(value[0] == 9);
    assert(value[0] == std_value[0]);
}

void test_vector_front_back(){
    c20_stl::vector<int> val(4, 6);
    std::vector<int> std_val(4, 6);

    for(c20_stl::vector<int>::size_type i = 0; i < val.size(); ++i){
        val[i] = static_cast<int>((i + 1) * 10);
        std_val[i] = val[i];
    }

    for(c20_stl::vector<int>::size_type i = 0; i < val.size(); ++i){
        assert(val[i] == std_val[i]);
        assert(val[i] == (i + 1) * 10);
    }

    static_assert(c20_stl::is_same_v<decltype(val.front()),
                                     decltype(std_val.front())>);
    static_assert(c20_stl::is_same_v<decltype(val.back()),
                                     decltype(std_val.back())>);
    static_assert(noexcept(val.front()) == noexcept(std_val.front()));
    static_assert(noexcept(val.back()) == noexcept(std_val.back()));

    const auto& const_val = val;
    const auto& std_const_val = std_val;

    static_assert(c20_stl::is_same_v<decltype(const_val.front()),
                                     decltype(std_const_val.front())>);
    static_assert(c20_stl::is_same_v<decltype(const_val.back()),
                                     decltype(std_const_val.back())>);
    static_assert(noexcept(const_val.front()) ==
                  noexcept(std_const_val.front()));
    static_assert(noexcept(const_val.back()) ==
                  noexcept(std_const_val.back()));

    assert(val.front() == std_val.front());
    assert(val.back() == std_val.back());

    val.front() = 100;
    std_val.front() = 100;
    val.back() = 400;
    std_val.back() = 400;

    assert(val[0] == 100);
    assert(val[3] == 400);
    assert(val[0] == std_val[0]);
    assert(val[3] == std_val[3]);
    assert(const_val.front() == std_const_val.front());
    assert(const_val.back() == std_const_val.back());

    c20_stl::vector<int> single(1, 7);
    std::vector<int> std_single(1, 7);

    assert(single.front() == std_single.front());
    assert(single.back() == std_single.back());
    assert(&single.front() == &single.back());
}

void test_vector_at(){
    c20_stl::vector<int> v{10, 20, 30, 40};
    std::vector<int> std_v{10, 20, 30, 40};

    static_assert(c20_stl::is_same_v<decltype(v.at(1)), decltype(std_v.at(1))>);

    assert(v.at(0) == 10);
    assert(v.at(2) == 30);
    assert(v.at(3) == 40);
    assert(v.at(0) == std_v.at(0));
    assert(v.at(2) == std_v.at(2));
    assert(v.at(3) == std_v.at(3));
}

void test_vector_at_out_of_range(){
    c20_stl::vector<int> value(3, 7);
    std::vector<int> std_value(3, 7);

    bool c20_threw = false;
    try{
        (void)value.at(value.size());
    }
    catch(const std::out_of_range&){
        c20_threw = true;
    }

    bool std_threw = false;
    try{
        (void)std_value.at(std_value.size());
    }
    catch(const std::out_of_range&){
        std_threw = true;
    }

    assert(c20_threw);
    assert(std_threw);
}


void test_vector_begin_end(){
    c20_stl::vector<int> value{10, 20, 30, 40};
    std::vector<int> std_value{10, 20, 30, 40};

    static_assert(c20_stl::is_same_v<
        decltype(*value.begin()),
        decltype(*std_value.begin())>);

    static_assert(noexcept(value.begin()) ==
                  noexcept(std_value.begin()));
    static_assert(noexcept(value.end()) ==
                  noexcept(std_value.end()));

    assert(value.end() - value.begin() ==
           std_value.end() - std_value.begin());
    assert(value.end() - value.begin() == value.size());
    assert(value.end() - value.begin() == 4);
    static_assert(c20_stl::is_same_v<
        decltype(value.end() - value.begin()), std::ptrdiff_t>);

    assert(*value.begin() == *std_value.begin());
    assert(*(value.end() - 1) == *(std_value.end() - 1));

    auto it = value.begin();
    auto std_it = std_value.begin();

    for(; it != value.end(); ++it, ++std_it){
        assert(*it == *std_it);
    }

    assert(std_it == std_value.end());

    *value.begin() = 100;
    *std_value.begin() = 100;

    assert(value.front() == 100);
    assert(value.front() == std_value.front());
}

}

int main() {
    c20_stl::test_vector_type();
    c20_stl::test_vector_default_constructor();
    c20_stl::test_vector_empty();
    c20_stl::test_vector_size();
    c20_stl::test_vector_capacity();
    c20_stl::test_vector_destroy();
    c20_stl::test_vector_fill_constructor();
    c20_stl::test_vector_fill_lifecycle();
    c20_stl::test_vector_fill_exception();
    c20_stl::test_vector_initializer_list_constructor();
    c20_stl::test_vector_initializer_list_exception();
    c20_stl::test_vector_data();
    c20_stl::test_vector_subscript();
    c20_stl::test_vector_front_back();

    c20_stl::test_vector_at();
    c20_stl::test_vector_at_out_of_range();
    c20_stl::test_vector_begin_end();
}
