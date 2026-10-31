# `typename`：模板类型参数与依赖类型消歧

`typename` 有两个常见用途：声明模板类型参数，以及告诉编译器某个依赖名称是类型。
`remove_volatile_t` 同时用到了这两种用途。

## 1. 从 remove_volatile_t 看两种用途

```cpp
template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;
```

- 第一个 `typename`：声明 `T` 是模板类型参数。
- 第二个 `typename`：声明 `remove_volatile<T>::type` 是一个类型。

第二个 `typename` 不参与类型转换，也不会产生运行时开销；它只是帮助编译器
解析模板代码。

## 2. 为什么编译器需要提示

定义模板时，`T` 尚未确定，因此 `remove_volatile<T>` 是一个**依赖类型**，
其中的 `type` 是一个**依赖限定名称**。

同样是类作用域里的 `type`，不同类可以给出完全不同的含义：

```cpp
template <typename T>
struct trait {
    using type = T;                  // type 是类型
};

template <>
struct trait<int> {
    static constexpr int type = 0;   // type 是静态数据成员
};
```

编译器需要先确定模板语句如何解析，不能等到每次实例化后再重新理解代码结构。
在需要消歧的上下文中，依赖限定名称默认按非类型处理；`typename` 明确告诉
编译器：“这里请按类型理解。”

## 3. 可编译示例

```cpp
#include <type_traits>

template <typename T>
struct remove_volatile {
    using type = T;
};

template <typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

static_assert(std::is_same_v<remove_volatile_t<volatile int>, int>);
static_assert(std::is_same_v<remove_volatile_t<const int>, const int>);
```

实例化 `remove_volatile_t<volatile int>` 时，名字的含义依次是：

```text
remove_volatile_t<volatile int>
→ remove_volatile<volatile int>::type
→ int
```

`typename` 只负责说明 `::type` 是类型；具体得到哪个类型，仍由模板实例化决定。

## 4. 非依赖名称不需要这种提示

如果模板参数已经全部确定，名称不再依赖未知参数，就不需要 `typename`：

```cpp
using result = remove_volatile<volatile int>::type;

static_assert(std::is_same_v<result, int>);
```

因此，需要关注的不是代码里有没有 `::`，而是 `::` 前面的内容是否依赖于
尚未确定的模板参数。

## 5. C++20 允许在部分上下文中省略

C++20 放宽了部分规则。例如，在别名声明的顶层类型说明符序列中，下面的写法合法：

```cpp
template <typename T>
struct box {
    using type = T;
};

template <typename T>
using box_t = box<T>::type;   // C++20：此上下文可以省略 typename
```

但不能由此推断“C++20 中都可以省略”。例如，在函数内声明变量时仍需消歧：

```cpp
template <typename T>
void f()
{
    typename box<T>::type value{};
    (void)value;
}
```

这里若省略 `typename`，编译器会报“需要 `typename`，因为 `box<T>` 是依赖作用域”。
项目中保留传统写法更直观，也能兼容 C++20 之前的标准。

## 6. 小结

| 写法 | `typename` 的作用 |
|---|---|
| `template <typename T>` | 声明模板类型参数 |
| `typename trait<T>::type` | 表明依赖限定名称 `type` 是类型 |
| `trait<int>::type` | 名称不依赖模板参数，不需要消歧 |

一句话记忆：**模板参数尚未确定时，用 `typename` 告诉编译器“这个依赖名称是类型”。**
