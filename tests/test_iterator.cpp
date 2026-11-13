#include <c20_stl/iterator.hpp>
#include <c20_stl/type_traits.hpp>
#include <cassert>
#include <iterator>
#include <string>
#include <type_traits>


namespace c20_stl{

void test_iterator_tag(){
    // 继承链:每一级都是上一级的子类
    static_assert(std::is_base_of_v<input_iterator_tag, forward_iterator_tag>);
    static_assert(std::is_base_of_v<forward_iterator_tag, bidirectional_iterator_tag>);
    static_assert(std::is_base_of_v<bidirectional_iterator_tag, random_access_iterator_tag>);
    static_assert(std::is_base_of_v<random_access_iterator_tag, contiguous_iterator_tag>);
    // 跨级传递性:input 是所有能力的"最弱祖先"
    static_assert(std::is_base_of_v<input_iterator_tag, random_access_iterator_tag>);
    static_assert(std::is_base_of_v<input_iterator_tag, contiguous_iterator_tag>);
    // 反向必须为 false(弱的不是强的基类)
    static_assert(!std::is_base_of_v<random_access_iterator_tag, input_iterator_tag>);
    static_assert(!std::is_base_of_v<contiguous_iterator_tag, random_access_iterator_tag>);

    // std 标签链同构对照(裁判)
    static_assert(std::is_base_of_v<std::input_iterator_tag, std::random_access_iterator_tag>);
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, std::contiguous_iterator_tag>);
}

// 用 iterator 基类造一个假迭代器,验证 traits 萃取
struct fake_random_access_iterator
    : iterator<random_access_iterator_tag, int> {};

void test_iterator_traits(){
    // 自定义类迭代器:五个成员类型原样萃取
    static_assert(std::is_same_v<iterator_traits<fake_random_access_iterator>::iterator_category,
                                 random_access_iterator_tag>);
    static_assert(std::is_same_v<iterator_traits<fake_random_access_iterator>::value_type, int>);
    static_assert(std::is_same_v<iterator_traits<fake_random_access_iterator>::difference_type,
                                 std::ptrdiff_t>);
    static_assert(std::is_same_v<iterator_traits<fake_random_access_iterator>::pointer, int*>);
    static_assert(std::is_same_v<iterator_traits<fake_random_access_iterator>::reference, int&>);

    // 内置指针特化
    static_assert(std::is_same_v<iterator_traits<int*>::iterator_category, random_access_iterator_tag>);
    static_assert(std::is_same_v<iterator_traits<int*>::value_type, int>);
    static_assert(std::is_same_v<iterator_traits<int*>::difference_type, std::ptrdiff_t>);
    static_assert(std::is_same_v<iterator_traits<int*>::pointer, int*>);
    static_assert(std::is_same_v<iterator_traits<int*>::reference, int&>);

    // const 指针特化:值类型剥 const,pointer/reference 保留 const
    static_assert(std::is_same_v<iterator_traits<const int*>::iterator_category, random_access_iterator_tag>);
    static_assert(std::is_same_v<iterator_traits<const int*>::value_type, int>);
    static_assert(std::is_same_v<iterator_traits<const int*>::pointer, const int*>);
    static_assert(std::is_same_v<iterator_traits<const int*>::reference, const int&>);

    // std 裁判:与 std::iterator_traits 的萃取结果一致
    static_assert(std::is_same_v<iterator_traits<int*>::value_type,
                                 std::iterator_traits<int*>::value_type>);
    static_assert(std::is_same_v<iterator_traits<const int*>::value_type,
                                 std::iterator_traits<const int*>::value_type>);
    static_assert(std::is_same_v<iterator_traits<int*>::difference_type,
                                 std::iterator_traits<int*>::difference_type>);
    static_assert(std::is_same_v<iterator_traits<const int*>::pointer,
                                 std::iterator_traits<const int*>::pointer>);
}

// 标签分发演示:重载决议"能者多劳"
std::string distance_dispatch(input_iterator_tag) { return "linear"; }
std::string distance_dispatch(random_access_iterator_tag) { return "O(1)"; }

void test_tag_dispatch(){
    // 精确匹配 random_access 版(零转换)
    assert(distance_dispatch(random_access_iterator_tag{}) == "O(1)");
    // contiguous 是 random_access 的子类:一级向上转型也命中 O(1) 版
    assert(distance_dispatch(contiguous_iterator_tag{}) == "O(1)");
    // bidirectional/forward 与 random_access 版无缘,只能向上转成 input 版
    assert(distance_dispatch(bidirectional_iterator_tag{}) == "linear");
    assert(distance_dispatch(forward_iterator_tag{}) == "linear");
    assert(distance_dispatch(input_iterator_tag{}) == "linear");
}

}

int main(){
    c20_stl::test_iterator_tag();
    c20_stl::test_iterator_traits();
    c20_stl::test_tag_dispatch();
}
