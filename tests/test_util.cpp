#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
#include <c20_stl/util.hpp>



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
    move_only_object& operator=(const move_only_object&) = delete;

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

}

int main(){
    c20_stl::test_move();
    c20_stl::test_forward();
}
