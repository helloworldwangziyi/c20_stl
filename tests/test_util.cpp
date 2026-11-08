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

}

int main(){
    c20_stl::test_move();
}
