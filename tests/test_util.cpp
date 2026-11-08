#include "c20_stl/memory.hpp"
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
#include <c20_stl/util.hpp>
#include <iostream>



namespace c20_stl{

class move_only_object
{
public:
    static inline int constructions = 0;
    static inline int destructions = 0;
    static inline int moves = 0;

    int val;

    explicit move_only_object(int v): val(v)
    {
        constructions++;
    };
    move_only_object(const move_only_object&) = delete;
    move_only_object& operator=(move_only_object&& other) noexcept{
        this->val = other.val;
        other.val = -1; // 清理移动之后的
        ++moves;
        return *this;
    }

    move_only_object(move_only_object&& other) noexcept: val(other.val)
    {
        other.val = -1; // 清理移动之后的值
        constructions++;
        moves++;
    }

    ~move_only_object()
    {
        destructions++;
    }
};

void test_move(){
    int x = 0;
    static_assert(std::is_same_v<decltype(c20_stl::move(x)), int&&>);
    static_assert(std::is_same_v<
    decltype(c20_stl::move(std::declval<const int&>())),
    const int&&>);
    static_assert(c20_stl::is_same_v<decltype(c20_stl::move(std::declval<int&&>())), int&&>);
    static_assert(noexcept(c20_stl::move(std::declval<int&>())));


    // 单独调用move不应移动资源
    move_only_object source{42};
    assert(move_only_object::constructions == 1);
    assert(move_only_object::moves == 0);

    auto &&result = c20_stl::move(source);
    assert(move_only_object::moves == 0);
    assert(&result == &source);
    assert(result.val == 42);

    // 用右值真正构造对象
    move_only_object obj{42};
    assert(move_only_object::moves == 0);

    move_only_object target(c20_stl::move(obj));
    assert(move_only_object::moves == 1);
    assert(target.val == 42);
    // 标准库对照
    static_assert(std::is_same_v<
        decltype(c20_stl::move(std::declval<int&>())),
        decltype(std::move(std::declval<int&>()))>);

    static_assert(std::is_same_v<
        decltype(c20_stl::move(std::declval<const int&>())),
        decltype(std::move(std::declval<const int&>()))>);

}

void test_forward(){
    // 计数器清零,本测试独立计
    move_only_object::constructions = 0;
    move_only_object::destructions = 0;
    move_only_object::moves = 0;

    // ---- 类型四格:与 std::forward 逐一裁判 ----
    int x = 0;
    static_assert(std::is_same_v<decltype(c20_stl::forward<int>(x)), int&&>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<int&>(x)), int&>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<const int&>(x)), const int&>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<int>(std::move(x))), int&&>);

    static_assert(std::is_same_v<decltype(c20_stl::forward<int>(x)),
                                 decltype(std::forward<int>(x))>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<int&>(x)),
                                 decltype(std::forward<int&>(x))>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<const int&>(x)),
                                 decltype(std::forward<const int&>(x))>);
    static_assert(std::is_same_v<decltype(c20_stl::forward<int>(std::move(x))),
                                 decltype(std::forward<int>(std::move(x)))>);

    // ---- noexcept ----
    static_assert(noexcept(c20_stl::forward<int>(x)));
    static_assert(noexcept(c20_stl::forward<int&>(x)));

    // ---- 行为:右值身份经 forward 还原,走移动构造 ----
    move_only_object obj{42};
    move_only_object target(c20_stl::forward<move_only_object>(obj));
    assert(move_only_object::moves == 1);
    assert(target.val == 42);
    assert(obj.val == -1);              // 资源已搬走

    // ---- 左值身份经 forward<T&> 保持,只是别名,不移动 ----
    move_only_object src{7};
    auto&& ref = c20_stl::forward<move_only_object&>(src);
    static_assert(std::is_same_v<decltype(ref), move_only_object&>);
    assert(move_only_object::moves == 1);   // 没有多出来的移动
    assert(&ref == &src);
    assert(src.val == 7);
}

void test_swap(){
    // 基本类型:值互换
    int a = 1;
    int b = 2;
    c20_stl::swap(a, b);
    assert(a == 2);
    assert(b == 1);

    // 仅移动类型:值互换 + 零拷贝对账
    move_only_object::constructions = 0;
    move_only_object::destructions = 0;
    move_only_object::moves = 0;
    move_only_object obj1{10};
    move_only_object obj2{20};
    c20_stl::swap(obj1, obj2);
    assert(obj1.val == 20);
    assert(obj2.val == 10);
    assert(move_only_object::moves == 3);           // tmp 移动构造(1) + 两次移动赋值(2)
    assert(move_only_object::constructions == 3);   // obj1/obj2 + tmp
    assert(move_only_object::destructions == 1);    // tmp 随 swap 返回已析构

    // 自交换:值不变,不崩
    c20_stl::swap(obj1, obj1);
    assert(obj1.val == 20);
}

void test_pair(){
    c20_stl::pair<int ,double>p1(23, 2.3);
    assert(p1.first == 23);
    assert(p1.second == 2.3);

    int x = 42;
    double y = 3.14;
    c20_stl::pair<int&, double&>p2(x, y);
    assert(p2.first == 42);
    assert(p2.second == 3.14);
    assert(&p2.first == &x);
    assert(&p2.second == &y);

    move_only_object::constructions = 0;                    
    move_only_object::destructions = 0;                     
    move_only_object::moves = 0;    

    move_only_object test{1};
    move_only_object test2{2};
    c20_stl::pair<move_only_object, move_only_object> p3(c20_stl::move(test), c20_stl::move(test2));
    assert(p3.first.val == 1);
    assert(p3.second.val == 2);
    assert(move_only_object::moves == 2);          // 两成员各移动一次

    {
        move_only_object test3{1};
        move_only_object test4{2};
        c20_stl::pair<move_only_object&, move_only_object&> p4(test3, test4);
        assert(p4.first.val == 1);
        assert(p4.second.val == 2);
        assert(&p4.first == &test3);
        assert(&p4.second == &test4);
    }   // test3/test4 在此析构(+2);p4 的引用成员析构是 no-op
    assert(move_only_object::destructions == 2);

}

}

int main(){
    c20_stl::test_move();
    c20_stl::test_forward();
    c20_stl::test_swap();
    c20_stl::test_pair();
}
