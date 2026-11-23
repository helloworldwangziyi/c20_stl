#include <cassert>
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

}
