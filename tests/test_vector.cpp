#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <c20_stl/vector.hpp>
#include <c20_stl/type_traits.hpp>
#include <type_traits>
#include <utility>
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

template<typename Lhs, typename Rhs>
void assert_same_elements(const Lhs& value, const Rhs& std_value){
    assert(value.size() == std_value.size());

    for(std::size_t i = 0; i < value.size(); ++i){
        assert(value[i] == std_value[i]);
    }
}

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
    static_assert(noexcept(v.at(0)) == noexcept(std_v.at(0)));

    const auto& const_v = v;
    const auto& std_const_v = std_v;

    static_assert(c20_stl::is_same_v<
        decltype(const_v.at(1)), decltype(std_const_v.at(1))>);
    static_assert(noexcept(const_v.at(0)) == noexcept(std_const_v.at(0)));

    assert(v.at(0) == 10);
    assert(v.at(2) == 30);
    assert(v.at(3) == 40);
    assert(v.at(0) == std_v.at(0));
    assert(v.at(2) == std_v.at(2));
    assert(v.at(3) == std_v.at(3));

    assert(const_v.at(0) == std_const_v.at(0));
    assert(const_v.at(2) == std_const_v.at(2));
    assert(const_v.at(3) == std_const_v.at(3));

    assert(&v.at(1) == &v[1]);
    assert(&std_v.at(1) == &std_v[1]);

    v.at(1) = 42;
    std_v.at(1) = 42;

    assert(v[1] == 42);
    assert(v[1] == std_v[1]);
    assert(const_v.at(1) == std_const_v.at(1));
}

void test_vector_at_out_of_range(){
    c20_stl::vector<int> value(3, 7);
    std::vector<int> std_value(3, 7);

    auto throws_out_of_range = [](auto& vec, std::size_t pos){
        try{
            (void)vec.at(pos);
        }
        catch(const std::out_of_range&){
            return true;
        }
        return false;
    };

    const auto original_size = value.size();
    const auto original_capacity = value.capacity();

    assert(throws_out_of_range(value, value.size()));
    assert(throws_out_of_range(std_value, std_value.size()));
    assert(throws_out_of_range(value, value.size() + 1));
    assert(throws_out_of_range(std_value, std_value.size() + 1));

    c20_stl::vector<int> empty_value;
    std::vector<int> std_empty_value;

    assert(throws_out_of_range(empty_value, 0));
    assert(throws_out_of_range(std_empty_value, 0));

    assert(value.size() == original_size);
    assert(value.capacity() == original_capacity);

    for(c20_stl::vector<int>::size_type i = 0; i < value.size(); ++i){
        assert(value[i] == std_value[i]);
    }
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
    static_assert(c20_stl::is_same_v<
        decltype(value.end() - value.begin()), std::ptrdiff_t>);

    const auto& const_value = value;
    const auto& std_const_value = std_value;

    static_assert(c20_stl::is_same_v<
        decltype(*const_value.begin()),
        decltype(*std_const_value.begin())>);
    static_assert(c20_stl::is_same_v<
        decltype(const_value.begin()), c20_stl::vector<int>::const_iterator>);
    static_assert(c20_stl::is_same_v<
        decltype(const_value.end()), c20_stl::vector<int>::const_iterator>);
    static_assert(noexcept(const_value.begin()) ==
                  noexcept(std_const_value.begin()));
    static_assert(noexcept(const_value.end()) ==
                  noexcept(std_const_value.end()));

    static_assert(std::is_assignable_v<decltype(*value.begin()), int> ==
                  std::is_assignable_v<decltype(*std_value.begin()), int>);
    static_assert(std::is_assignable_v<decltype(*const_value.begin()), int> ==
                  std::is_assignable_v<decltype(*std_const_value.begin()), int>);

    assert(value.end() - value.begin() ==
           std_value.end() - std_value.begin());
    assert(value.end() - value.begin() == value.size());
    assert(value.end() - value.begin() == 4);
    assert(const_value.end() - const_value.begin() ==
           std_const_value.end() - std_const_value.begin());

    assert(*value.begin() == *std_value.begin());
    assert(*(value.end() - 1) == *(std_value.end() - 1));

    auto it = value.begin();
    auto std_it = std_value.begin();

    for(; it != value.end(); ++it, ++std_it){
        assert(*it == *std_it);
    }

    assert(it == value.end());
    assert(std_it == std_value.end());

    auto const_it = const_value.begin();
    auto std_const_it = std_const_value.begin();

    for(; const_it != const_value.end(); ++const_it, ++std_const_it){
        assert(*const_it == *std_const_it);
    }

    assert(const_it == const_value.end());
    assert(std_const_it == std_const_value.end());

    std::size_t count = 0;
    int sum = 0;
    for(int element : value){
        ++count;
        sum += element;
    }

    std::size_t std_count = 0;
    int std_sum = 0;
    for(int element : std_value){
        ++std_count;
        std_sum += element;
    }

    assert(count == value.size());
    assert(count == std_count);
    assert(sum == std_sum);

    *value.begin() = 100;
    *std_value.begin() = 100;

    assert(value.front() == 100);
    assert(value.front() == std_value.front());

    c20_stl::vector<int> single{7};
    std::vector<int> std_single{7};

    assert(single.begin() != single.end());
    assert(single.end() - single.begin() == 1);
    assert(single.end() - single.begin() ==
           std_single.end() - std_single.begin());

    std::size_t single_count = 0;
    for(auto single_it = single.begin(); single_it != single.end(); ++single_it){
        ++single_count;
        assert(*single_it == *std_single.begin());
    }
    assert(single_count == 1);

    c20_stl::vector<int> empty_value;
    std::vector<int> std_empty_value;

    assert(empty_value.begin() == empty_value.end());
    assert(std_empty_value.begin() == std_empty_value.end());

    const auto& const_empty_value = empty_value;
    assert(const_empty_value.begin() == const_empty_value.end());
}

void test_vector_const_iterators(){
    c20_stl::vector<int> value{10, 20, 30, 40};
    std::vector<int> std_value{10, 20, 30, 40};

    static_assert(c20_stl::is_same_v<
        decltype(value.cbegin()), c20_stl::vector<int>::const_iterator>);
    static_assert(c20_stl::is_same_v<
        decltype(value.cend()), c20_stl::vector<int>::const_iterator>);
    static_assert(c20_stl::is_same_v<
        decltype(*value.cbegin()), decltype(*std_value.cbegin())>);
    static_assert(noexcept(value.cbegin()) == noexcept(std_value.cbegin()));
    static_assert(noexcept(value.cend()) == noexcept(std_value.cend()));

    static_assert(std::is_assignable_v<decltype(*value.cbegin()), int> ==
                  std::is_assignable_v<decltype(*std_value.cbegin()), int>);

    assert(value.cend() - value.cbegin() == value.size());
    assert(value.cend() - value.cbegin() ==
           std_value.cend() - std_value.cbegin());
    assert(*value.cbegin() == *std_value.cbegin());
    assert(*(value.cend() - 1) == *(std_value.cend() - 1));

    auto it = value.cbegin();
    auto std_it = std_value.cbegin();

    for(; it != value.cend(); ++it, ++std_it){
        assert(*it == *std_it);
    }

    assert(it == value.cend());
    assert(std_it == std_value.cend());

    c20_stl::vector<int> empty_value;
    std::vector<int> std_empty_value;

    assert(empty_value.cbegin() == empty_value.cend());
    assert(std_empty_value.cbegin() == std_empty_value.cend());
}

void test_vector_swap(){
    c20_stl::vector<int> value{1, 2};
    c20_stl::vector<int> other{10, 20, 30};
    std::vector<int> std_value{1, 2};
    std::vector<int> std_other{10, 20, 30};

    static_assert(noexcept(value.swap(other)) ==
                  noexcept(std_value.swap(std_other)));

    auto* value_data = value.data();
    auto* other_data = other.data();

    value.swap(other);
    std_value.swap(std_other);

    assert(value.data() == other_data);
    assert(other.data() == value_data);
    assert_same_elements(value, std_value);
    assert_same_elements(other, std_other);

    value.swap(value);
    std_value.swap(std_value);
    assert_same_elements(value, std_value);

    c20_stl::vector<int> empty_value;
    c20_stl::vector<int> single{7};
    std::vector<int> std_empty_value;
    std::vector<int> std_single{7};

    empty_value.swap(single);
    std_empty_value.swap(std_single);

    assert_same_elements(empty_value, std_empty_value);
    assert(single.empty());
    assert(std_single.empty());
}

void test_vector_move_constructor(){
    static_assert(std::is_nothrow_move_constructible_v<c20_stl::vector<int>> ==
                  std::is_nothrow_move_constructible_v<std::vector<int>>);

    c20_stl::vector<int> source{10, 20, 30};
    std::vector<int> std_source{10, 20, 30};

    auto* source_data = source.data();
    const auto source_size = source.size();
    const auto source_capacity = source.capacity();

    c20_stl::vector<int> value(c20_stl::move(source));
    std::vector<int> std_value(std::move(std_source));

    assert(value.data() == source_data);
    assert(value.size() == source_size);
    assert(value.capacity() == source_capacity);
    assert(source.empty());
    assert(source.data() == nullptr);
    assert_same_elements(value, std_value);

    c20_stl::vector<int> empty_source;
    c20_stl::vector<int> empty_value(c20_stl::move(empty_source));

    assert(empty_value.empty());
    assert(empty_value.data() == nullptr);
    assert(empty_source.empty());

    Item::constructions = 0;
    Item::destructions = 0;
    {
        c20_stl::vector<Item> item_source{Item(1), Item(2)};
        const auto constructions_before_move = Item::constructions;

        c20_stl::vector<Item> item_value(c20_stl::move(item_source));

        assert(Item::constructions == constructions_before_move);
        assert(item_source.empty());
    }
    assert(Item::constructions == Item::destructions);
}

void test_vector_copy_constructor(){
    static_assert(std::is_copy_constructible_v<c20_stl::vector<int>> ==
                  std::is_copy_constructible_v<std::vector<int>>);
    static_assert(std::is_nothrow_copy_constructible_v<c20_stl::vector<int>> ==
                  std::is_nothrow_copy_constructible_v<std::vector<int>>);

    const c20_stl::vector<int> source{10, 20, 30};
    const std::vector<int> std_source{10, 20, 30};

    c20_stl::vector<int> value(source);
    std::vector<int> std_value(std_source);

    assert(value.data() != source.data());
    assert(std_value.data() != std_source.data());
    assert_same_elements(value, std_value);

    assert(source[0] == 10);
    assert(source[1] == 20);
    assert(source[2] == 30);

    value[0] = 99;
    std_value[0] = 99;

    assert(source[0] == 10);
    assert_same_elements(value, std_value);

    c20_stl::vector<int> empty_source;
    c20_stl::vector<int> empty_value(empty_source);

    assert(empty_value.empty());
    assert(empty_value.data() == nullptr);
}

void test_vector_move_assignment(){
    static_assert(std::is_nothrow_move_assignable_v<c20_stl::vector<int>> ==
                  std::is_nothrow_move_assignable_v<std::vector<int>>);

    c20_stl::vector<int> target{1, 2};
    c20_stl::vector<int> source{10, 20, 30};
    std::vector<int> std_target{1, 2};
    std::vector<int> std_source{10, 20, 30};

    static_assert(c20_stl::is_same_v<
        decltype(target = c20_stl::move(source)), c20_stl::vector<int>&>);

    auto* source_data = source.data();
    auto& result = (target = c20_stl::move(source));
    auto& std_result = (std_target = std::move(std_source));

    assert(&result == &target);
    assert(&std_result == &std_target);
    assert(target.data() == source_data);
    assert(source.empty());
    assert(source.data() == nullptr);
    assert_same_elements(target, std_target);

    auto* target_data = target.data();
    target = c20_stl::move(target);

    assert(target.data() == target_data);
    assert_same_elements(target, std_target);

    c20_stl::vector<int> empty_source;
    std::vector<int> std_empty_source;

    target = c20_stl::move(empty_source);
    std_target = std::move(std_empty_source);

    assert(target.empty());
    assert(target.data() == nullptr);
    assert(std_target.empty());

    Item::constructions = 0;
    Item::destructions = 0;
    {
        c20_stl::vector<Item> item_target{Item(1), Item(2)};
        c20_stl::vector<Item> item_source{Item(3)};
        const auto constructions_before_move = Item::constructions;

        item_target = c20_stl::move(item_source);

        assert(Item::constructions == constructions_before_move);
        assert(item_source.empty());
    }
    assert(Item::constructions == Item::destructions);
}

void test_vector_copy_assignment(){
    static_assert(std::is_copy_assignable_v<c20_stl::vector<int>> ==
                  std::is_copy_assignable_v<std::vector<int>>);
    static_assert(std::is_nothrow_copy_assignable_v<c20_stl::vector<int>> ==
                  std::is_nothrow_copy_assignable_v<std::vector<int>>);

    c20_stl::vector<int> target{1, 2};
    c20_stl::vector<int> source{10, 20, 30};
    std::vector<int> std_target{1, 2};
    std::vector<int> std_source{10, 20, 30};

    static_assert(c20_stl::is_same_v<
        decltype(target = source), c20_stl::vector<int>&>);

    auto& result = (target = source);
    auto& std_result = (std_target = std_source);

    assert(&result == &target);
    assert(&std_result == &std_target);
    assert(target.data() != source.data());
    assert(std_target.data() != std_source.data());
    assert_same_elements(target, std_target);

    target[0] = 100;
    std_target[0] = 100;

    assert(source[0] == 10);
    assert_same_elements(target, std_target);

    target = target;
    std_target = std_target;
    assert_same_elements(target, std_target);

    c20_stl::vector<int> empty_source;
    std::vector<int> std_empty_source;

    target = empty_source;
    std_target = std_empty_source;

    assert(target.empty());
    assert(target.data() == nullptr);
    assert(std_target.empty());
}

void test_vector_copy_exception(){
    ThrowingItem::alive = 0;
    {
        ThrowingItem other;

        ThrowingItem::copies_before_throw = 5;
        c20_stl::vector<ThrowingItem> source(5, other);

        ThrowingItem::copies_before_throw = 5;
        std::vector<ThrowingItem> std_source(5, other);

        const int source_alive = ThrowingItem::alive;

        ThrowingItem::copies_before_throw = 2;
        bool c20_copy_threw = false;
        try{
            c20_stl::vector<ThrowingItem> copy(source);
        }
        catch(int){
            c20_copy_threw = true;
        }

        assert(c20_copy_threw);
        assert(ThrowingItem::alive == source_alive);

        ThrowingItem::copies_before_throw = 2;
        bool std_copy_threw = false;
        try{
            std::vector<ThrowingItem> std_copy(std_source);
        }
        catch(int){
            std_copy_threw = true;
        }

        assert(std_copy_threw);
        assert(ThrowingItem::alive == source_alive);

        ThrowingItem::copies_before_throw = 2;
        c20_stl::vector<ThrowingItem> target(2, other);

        ThrowingItem::copies_before_throw = 2;
        std::vector<ThrowingItem> std_target(2, other);

        const int target_alive = ThrowingItem::alive;
        auto* target_data = target.data();
        const auto target_size = target.size();
        const auto target_capacity = target.capacity();

        ThrowingItem::copies_before_throw = 2;
        bool c20_assign_threw = false;
        try{
            target = source;
        }
        catch(int){
            c20_assign_threw = true;
        }

        assert(c20_assign_threw);
        assert(ThrowingItem::alive == target_alive);
        assert(target.data() == target_data);
        assert(target.size() == target_size);
        assert(target.capacity() == target_capacity);

        ThrowingItem::copies_before_throw = 2;
        bool std_assign_threw = false;
        try{
            std_target = std_source;
        }
        catch(int){
            std_assign_threw = true;
        }

        assert(std_assign_threw);
    }
    assert(ThrowingItem::alive == 0);
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
    c20_stl::test_vector_const_iterators();
    c20_stl::test_vector_swap();
    c20_stl::test_vector_move_constructor();
    c20_stl::test_vector_copy_constructor();
    c20_stl::test_vector_move_assignment();
    c20_stl::test_vector_copy_assignment();
    c20_stl::test_vector_copy_exception();
}
