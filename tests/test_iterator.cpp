#include <c20_stl/iterator.hpp>
#include <c20_stl/type_traits.hpp>
#include <cassert>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>


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

void test_forward_iterator(){
    class test_forward_iterator : public c20_stl::iterator<c20_stl::forward_iterator_tag, int>{
        private: int * current_;
        public:
            explicit test_forward_iterator(int* p) : current_(p){}

            bool operator!=(const test_forward_iterator& other) {
                return other.current_ != current_;
            }

            test_forward_iterator& operator++(){
                current_++;
                return *this;
            }

            int get_current_value(){
                return *(this->current_);
            }
    };

    int arr[] = {10, 20, 30, 40, 50};
    test_forward_iterator first(arr);
    test_forward_iterator last(arr + 5);

    assert(c20_stl::distance(first, last) == 5);

    test_forward_iterator advance_first(arr);
    int* std_first = arr;
    c20_stl::advance(advance_first, 3);
    std::advance(std_first, 3);
    assert(advance_first.get_current_value() == *std_first);

}

void test_input_iterator(){
    class test_input_iterator
        : public c20_stl::iterator<c20_stl::input_iterator_tag, int> {
        private:
            int* current_;
        public:
            explicit test_input_iterator(int* p) : current_(p) {}

            test_input_iterator& operator++() {
                ++current_;
                return *this;
            }

            int* base() const {
                return current_;
            }
    };

    int arr[] = {10, 20, 30, 40, 50};
    test_input_iterator first(arr);
    int* std_first = arr;

    c20_stl::advance(first, 2);
    std::advance(std_first, 2);
    assert(first.base() == std_first);
}

static_assert(std::is_same_v<
    decltype(c20_stl::advance(std::declval<int*&>(), 1)),
    void>);

void test_bidirectional_iterator(){
    class test_bidirectional_iterator : public c20_stl::iterator<c20_stl::bidirectional_iterator_tag, int>{
        private:
            int * current_;
        public:
            explicit test_bidirectional_iterator(int* p) : current_(p){}

            test_bidirectional_iterator& operator++(){
                ++current_;
                return *this;
            }

            test_bidirectional_iterator& operator--(){
                --current_;
                return *this;
            }

            int* base(){
                return current_;
            }
    };

    int arr[] = {1, 2, 3, 4, 5};
    test_bidirectional_iterator first(arr + 2);
    int * std_first = arr + 2;

    c20_stl::advance(first, 2);
    std::advance(std_first, 2);
    assert(first.base() == std_first);

    c20_stl::advance(first, -1);
    std::advance(std_first, -1);
    assert(first.base() == std_first);

}

void test_random_access_iterator(){
    int arr[] = {1, 2, 3, 4, 5};
    int* first = arr + 1;
    int* std_first = arr + 1;

    c20_stl::advance(first, 3);
    std::advance(std_first, 3);
    assert(first == std_first);

    c20_stl::advance(first, -2);
    std::advance(std_first, -2);
    assert(first == std_first);
}

void test_distance(){
    int arr[] = {10, 20, 30, 40, 50};
    assert(c20_stl::distance(arr, arr + 5) == std::distance(arr, arr + 5));
    assert(c20_stl::distance(arr, arr + 5) == 5);
}

void test_reverse_iterator(){
    using c20_reverse_iterator = c20_stl::reverse_iterator<int*>;
    using std_reverse_iterator = std::reverse_iterator<int*>;

    // 类型萃取与构造
    static_assert(std::is_same_v<c20_reverse_iterator::iterator_type,
                                 std_reverse_iterator::iterator_type>);
    static_assert(std::is_same_v<c20_reverse_iterator::value_type,
                                 std_reverse_iterator::value_type>);
    static_assert(std::is_same_v<c20_reverse_iterator::difference_type,
                                 std_reverse_iterator::difference_type>);
    static_assert(std::is_same_v<c20_reverse_iterator::pointer,
                                 std_reverse_iterator::pointer>);
    static_assert(std::is_same_v<c20_reverse_iterator::reference,
                                 std_reverse_iterator::reference>);
    static_assert(std::is_same_v<c20_reverse_iterator::iterator_category,
                                 c20_stl::random_access_iterator_tag>);
    static_assert(std::is_same_v<std_reverse_iterator::iterator_category,
                                 std::random_access_iterator_tag>);

    c20_reverse_iterator c20_default;
    std_reverse_iterator std_default;
    assert(c20_default.base() == std_default.base());
    assert(c20_default.base() == nullptr);

    int arr[] = {1, 2, 3, 4, 5, 6};
    c20_reverse_iterator reverse_it(arr + 4);
    std_reverse_iterator std_reverse_it(arr + 4);
    static_assert(std::is_same_v<decltype(reverse_it.base()),
                                 decltype(std_reverse_it.base())>);
    assert(reverse_it.base() == std_reverse_it.base());
    assert(reverse_it.base() == &arr[4]);
    assert(*reverse_it == *std_reverse_it);
    assert(*reverse_it == 4);

    // 后置 ++/-- 返回旧位置，原对象移动一步
    auto c20_old = reverse_it++;
    auto std_old = std_reverse_it++;
    assert(c20_old.base() == std_old.base());
    assert(*c20_old == *std_old);
    assert(*c20_old == 4);
    assert(reverse_it.base() == std_reverse_it.base());
    assert(*reverse_it == *std_reverse_it);
    assert(*reverse_it == 3);

    c20_old = reverse_it--;
    std_old = std_reverse_it--;
    assert(c20_old.base() == std_old.base());
    assert(*c20_old == *std_old);
    assert(*c20_old == 3);
    assert(reverse_it.base() == std_reverse_it.base());
    assert(*reverse_it == *std_reverse_it);
    assert(*reverse_it == 4);

    // 前置 ++/-- 返回原对象的引用
    auto& c20_pre_increment = ++reverse_it;
    auto& std_pre_increment = ++std_reverse_it;
    assert(&c20_pre_increment == &reverse_it);
    assert(&std_pre_increment == &std_reverse_it);
    assert(c20_pre_increment.base() == std_pre_increment.base());
    assert(*c20_pre_increment == *std_pre_increment);
    assert(*c20_pre_increment == 3);

    auto& c20_pre_decrement = --reverse_it;
    auto& std_pre_decrement = --std_reverse_it;
    assert(&c20_pre_decrement == &reverse_it);
    assert(&std_pre_decrement == &std_reverse_it);
    assert(c20_pre_decrement.base() == std_pre_decrement.base());
    assert(*c20_pre_decrement == *std_pre_decrement);
    assert(*c20_pre_decrement == 4);

    // + 和 - 返回新迭代器，不修改原对象
    const auto c20_plus_zero = reverse_it + 0;
    const auto std_plus_zero = std_reverse_it + 0;
    assert(c20_plus_zero.base() == std_plus_zero.base());
    assert(*c20_plus_zero == *std_plus_zero);
    assert(*c20_plus_zero == 4);

    const auto c20_minus_zero = reverse_it - 0;
    const auto std_minus_zero = std_reverse_it - 0;
    assert(c20_minus_zero.base() == std_minus_zero.base());
    assert(*c20_minus_zero == *std_minus_zero);
    assert(*c20_minus_zero == 4);

    const auto c20_plus_two = reverse_it + 2;
    const auto std_plus_two = std_reverse_it + 2;
    assert(c20_plus_two.base() == std_plus_two.base());
    assert(*c20_plus_two == *std_plus_two);
    assert(*c20_plus_two == 2);

    const auto c20_minus_two = reverse_it - 2;
    const auto std_minus_two = std_reverse_it - 2;
    assert(c20_minus_two.base() == std_minus_two.base());
    assert(*c20_minus_two == *std_minus_two);
    assert(*c20_minus_two == 6);
    assert(reverse_it.base() == std_reverse_it.base());
    assert(reverse_it.base() == arr + 4);

    // += 和 -= 返回原对象的引用
    auto& c20_plus_assign = (reverse_it += 2);
    auto& std_plus_assign = (std_reverse_it += 2);
    assert(&c20_plus_assign == &reverse_it);
    assert(&std_plus_assign == &std_reverse_it);
    assert(c20_plus_assign.base() == std_plus_assign.base());
    assert(*c20_plus_assign == *std_plus_assign);
    assert(*c20_plus_assign == 2);

    auto& c20_minus_assign = (reverse_it -= 4);
    auto& std_minus_assign = (std_reverse_it -= 4);
    assert(&c20_minus_assign == &reverse_it);
    assert(&std_minus_assign == &std_reverse_it);
    assert(c20_minus_assign.base() == std_minus_assign.base());
    assert(*c20_minus_assign == *std_minus_assign);
    assert(*c20_minus_assign == 6);

    // 下标访问不修改 base()
    assert(reverse_it[0] == std_reverse_it[0]);
    assert(reverse_it[0] == 6);
    assert(reverse_it[4] == std_reverse_it[4]);
    assert(reverse_it[4] == 2);
    assert(reverse_it[5] == std_reverse_it[5]);
    assert(reverse_it[5] == 1);
    assert(reverse_it.base() == std_reverse_it.base());
    assert(reverse_it.base() == arr + 6);

    // 箭头访问的是反向解引用后的元素
    struct test_item {
        int value;
    };
    test_item items[] = {{10}, {20}, {30}};
    c20_stl::reverse_iterator<test_item*> c20_arrow(items + 2);
    std::reverse_iterator<test_item*> std_arrow(items + 2);
    static_assert(std::is_same_v<decltype(c20_arrow.operator->()),
                                 decltype(std_arrow.operator->())>);
    assert(c20_arrow.operator->() == std_arrow.operator->());
    assert(c20_arrow.operator->() == &items[1]);
    assert(c20_arrow->value == std_arrow->value);
    assert(c20_arrow->value == 20);

    // 比较运算：正反方向与相等位置都和 std 对照
    auto check_comparisons = [](const c20_reverse_iterator& c20_a,
                                const c20_reverse_iterator& c20_b,
                                const std_reverse_iterator& std_a,
                                const std_reverse_iterator& std_b) {
        assert((c20_a == c20_b) == (std_a == std_b));
        assert((c20_a < c20_b) == (std_a < std_b));
        assert((c20_a > c20_b) == (std_a > std_b));
        assert((c20_a <= c20_b) == (std_a <= std_b));
        assert((c20_a >= c20_b) == (std_a >= std_b));
    };

    c20_reverse_iterator c20_left(arr + 4);
    c20_reverse_iterator c20_right(arr + 2);
    c20_reverse_iterator c20_equal(arr + 4);
    std_reverse_iterator std_left(arr + 4);
    std_reverse_iterator std_right(arr + 2);
    std_reverse_iterator std_equal(arr + 4);

    check_comparisons(c20_left, c20_right, std_left, std_right);
    check_comparisons(c20_right, c20_left, std_right, std_left);
    check_comparisons(c20_left, c20_equal, std_left, std_equal);
    assert(!(c20_left == c20_right));
    assert(c20_left < c20_right);
    assert(!(c20_left > c20_right));
    assert(c20_left <= c20_right);
    assert(!(c20_left >= c20_right));
    assert(c20_left == c20_equal);
    assert(c20_left <= c20_equal);
    assert(c20_left >= c20_equal);

    // 两个反向迭代器相减：距离方向与 std 一致
    assert((c20_left - c20_right) == (std_left - std_right));
    assert(c20_left - c20_right == -2);
    assert((c20_right - c20_left) == (std_right - std_left));
    assert(c20_right - c20_left == 2);
    assert((c20_left - c20_equal) == (std_left - std_equal));
    assert(c20_left - c20_equal == 0);

    // n + it 与 it + n 一致，且不修改原对象
    const auto c20_n_plus = 2 + c20_left;
    const auto std_n_plus = 2 + std_left;
    assert(c20_n_plus.base() == std_n_plus.base());
    assert(*c20_n_plus == *std_n_plus);
    assert(*c20_n_plus == 2);
    assert(c20_n_plus.base() == (c20_left + 2).base());
    assert(c20_left.base() == std_left.base());
    assert(c20_left.base() == arr + 4);

    // 普通迭代器可以隐式转换为 const 迭代器，反向转换必须被拒绝
    using c20_const_reverse_iterator = c20_stl::reverse_iterator<const int*>;
    using std_const_reverse_iterator = std::reverse_iterator<const int*>;
    static_assert(std::is_convertible_v<c20_reverse_iterator,
                                        c20_const_reverse_iterator> ==
                  std::is_convertible_v<std_reverse_iterator,
                                        std_const_reverse_iterator>);
    static_assert(std::is_convertible_v<c20_reverse_iterator,
                                        c20_const_reverse_iterator>);
    static_assert(std::is_convertible_v<c20_const_reverse_iterator,
                                        c20_reverse_iterator> ==
                  std::is_convertible_v<std_const_reverse_iterator,
                                        std_reverse_iterator>);
    static_assert(!std::is_convertible_v<c20_const_reverse_iterator,
                                         c20_reverse_iterator>);

    c20_const_reverse_iterator c20_converted = c20_left;
    std_const_reverse_iterator std_converted = std_left;
    assert(c20_converted.base() == std_converted.base());
    assert(c20_converted.base() == arr + 4);
    assert(*c20_converted == *std_converted);
    assert(*c20_converted == 4);

    // 跨类型比较覆盖两个操作数方向
    c20_const_reverse_iterator c20_const_right = c20_right;
    std_const_reverse_iterator std_const_right = std_right;
    auto check_mixed_comparisons = [](const auto& c20_a,
                                      const auto& c20_b,
                                      const auto& std_a,
                                      const auto& std_b) {
        assert((c20_a == c20_b) == (std_a == std_b));
        assert((c20_a != c20_b) == (std_a != std_b));
        assert((c20_a < c20_b) == (std_a < std_b));
        assert((c20_a > c20_b) == (std_a > std_b));
        assert((c20_a <= c20_b) == (std_a <= std_b));
        assert((c20_a >= c20_b) == (std_a >= std_b));
    };
    check_mixed_comparisons(c20_left, c20_const_right,
                            std_left, std_const_right);
    check_mixed_comparisons(c20_const_right, c20_left,
                            std_const_right, std_left);
    assert((c20_left - c20_const_right) == (std_left - std_const_right));
    assert(c20_left - c20_const_right == -2);
    assert((c20_const_right - c20_left) == (std_const_right - std_left));
    assert(c20_const_right - c20_left == 2);

    // reverse_iterator 与 distance 协作
    c20_reverse_iterator c20_rbegin(arr + 6);
    c20_reverse_iterator c20_rend(arr);
    std_reverse_iterator std_rbegin(arr + 6);
    std_reverse_iterator std_rend(arr);
    const auto c20_reverse_distance = c20_stl::distance(c20_rbegin, c20_rend);
    const auto std_reverse_distance = std::distance(std_rbegin, std_rend);
    static_assert(std::is_same_v<decltype(c20_reverse_distance),
                                 decltype(std_reverse_distance)>);
    assert(c20_reverse_distance == std_reverse_distance);
    assert(c20_reverse_distance == 6);
    assert(c20_stl::distance(c20_rbegin, c20_rbegin) ==
           std::distance(std_rbegin, std_rbegin));
    assert(c20_stl::distance(c20_rbegin, c20_rbegin) == 0);

    // reverse_iterator 与 advance 协作，支持正负位移
    auto c20_advanced = c20_rbegin;
    auto std_advanced = std_rbegin;
    c20_stl::advance(c20_advanced, 3);
    std::advance(std_advanced, 3);
    assert(c20_advanced.base() == std_advanced.base());
    assert(*c20_advanced == *std_advanced);
    assert(*c20_advanced == 3);

    c20_stl::advance(c20_advanced, -2);
    std::advance(std_advanced, -2);
    assert(c20_advanced.base() == std_advanced.base());
    assert(*c20_advanced == *std_advanced);
    assert(*c20_advanced == 5);
}

}

int main(){
    c20_stl::test_iterator_tag();
    c20_stl::test_iterator_traits();
    c20_stl::test_tag_dispatch();
    c20_stl::test_input_iterator();
    c20_stl::test_forward_iterator();
    c20_stl::test_bidirectional_iterator();
    c20_stl::test_random_access_iterator();
    c20_stl::test_distance();
    c20_stl::test_reverse_iterator();
}
