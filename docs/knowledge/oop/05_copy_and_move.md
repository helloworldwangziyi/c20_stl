# 第 5 章：拷贝、移动与特殊成员函数

拷贝决定如何得到独立副本，移动决定如何转移资源；析构与这两个操作必须围绕同一种
所有权语义设计，否则就会出现重复释放、浅拷贝或无意义复制。

## 1. 六个特殊成员函数

一个类最受编译器关注的特殊成员包括：

- 默认构造函数。
- 析构函数。
- 拷贝构造函数。
- 拷贝赋值运算符。
- 移动构造函数。
- 移动赋值运算符。

拷贝、移动的典型声明形状如下：

```cpp
class widget {
public:
    widget(const widget& other);             // 拷贝构造
    widget& operator=(const widget& other);  // 拷贝赋值
    widget(widget&& other) noexcept;         // 移动构造
    widget& operator=(widget&& other) noexcept;
};
```

`noexcept` 不是语法必需品，但确实不抛异常的移动操作应明确标注，标准容器会根据它
选择更合适的拷贝或移动策略。

## 2. 拷贝构造与拷贝赋值的区别

```cpp
widget a;
widget b = a;   // 拷贝构造：创建 b
widget c;
c = a;          // 拷贝赋值：a 的内容赋给已经存在的 c
```

构造函数面对的是尚未存在的当前对象，赋值运算符面对的是已经持有状态的当前对象。
因此赋值还要考虑释放旧资源、自赋值以及异常发生后的状态。

`widget b(a);`、`widget b = a;` 都可能调用拷贝构造；后者不是赋值，因为 `b` 正在创建。

## 3. 深拷贝与浅拷贝

**浅拷贝**逐位或逐成员复制表层状态。如果两个对象因此指向同一份需要各自释放的资源，
析构时就可能重复释放。

**深拷贝**为副本创建独立资源，使两个对象的生命周期互不影响。`std::vector` 的拷贝就是
典型的深拷贝：复制元素，而不是让两个 vector 共享同一块元素存储。

```cpp
#include <vector>

std::vector<int> a{1, 2, 3};
std::vector<int> b = a;
b[0] = 99;
// a[0] 仍为 1，两个 vector 拥有独立元素存储
```

自定义类如果直接用 `vector`、`string` 管理资源，默认逐成员拷贝往往已经具有正确语义，
无需再手写一套指针复制逻辑。

## 4. 移动构造与移动赋值

移动操作接收右值引用参数，可以接管来源对象的资源，而不必完整复制：

```cpp
class widget {
public:
    widget(widget&& other) noexcept;
    widget& operator=(widget&& other) noexcept;
};
```

`std::move(a)` 只是允许选择移动操作，真正做什么由移动构造或移动赋值决定：

```cpp
widget b = std::move(a);  // 移动构造
widget c;
c = std::move(b);         // 移动赋值
```

移动后的来源对象必须仍然有效、可析构，也通常应可重新赋值；但除非具体接口另有保证，
不要依赖它仍保有原值。标准库经常把这一状态称为“有效但未指定”。

## 5. Rule of Three 与 Rule of Five

如果一个类需要自定义析构函数、拷贝构造或拷贝赋值中的任意一个，往往需要同时审视
这三个操作，这叫 **Rule of Three**。加入移动语义后，扩展为 **Rule of Five**：

```text
析构 + 拷贝构造 + 拷贝赋值 + 移动构造 + 移动赋值
```

这不是“五个函数必须全部手写”的硬性规定，而是设计提醒：自定义其中之一通常说明类
具有特殊资源语义，其余操作也必须与该语义一致。

例如，只写析构释放裸指针，却保留默认浅拷贝，就很容易造成双重释放。

## 6. Rule of Zero：优先不直接管理资源

更理想的设计是让资源管理交给专门的成员：

```cpp
#include <string>
#include <vector>

struct profile {
    std::string name;
    std::vector<int> scores;
};
```

`profile` 不需要手写析构、拷贝或移动。`string` 与 `vector` 已经分别管理好自己的资源，
编译器生成的逐成员特殊成员函数会组合出正确行为。

可以把职责分开：

- **资源管理类**：少数类专门处理所有权，认真设计五个操作。
- **普通业务类**：使用 RAII 成员，尽量遵守 Rule of Zero。

## 7. 可编译示例：观察调用了哪个操作

```cpp
#include <cassert>
#include <utility>

struct probe {
    static inline int copies = 0;
    static inline int moves = 0;
    static inline int destructions = 0;

    probe() = default;

    probe(const probe&) { ++copies; }

    probe& operator=(const probe&)
    {
        ++copies;
        return *this;
    }

    probe(probe&&) noexcept { ++moves; }

    probe& operator=(probe&&) noexcept
    {
        ++moves;
        return *this;
    }

    ~probe() { ++destructions; }
};

int main()
{
    probe a;
    probe b = a;
    probe c;
    c = b;
    probe d = std::move(a);

    assert(probe::copies == 2);
    assert(probe::moves == 1);

    const int before = probe::destructions;
    {
        probe temporary;
    }
    assert(probe::destructions == before + 1);
}
```

这个类型没有真实资源，只用于观察特殊成员函数。换成资源管理类时，每个操作还必须正确
处理资源所有权。

## 8. 本章要点

1. 构造函数创建新对象，赋值运算符修改已经存在的对象。
2. 拷贝追求独立副本，移动转移资源，但移动后来源对象仍须有效。
3. Rule of Three / Five 要求围绕同一种所有权语义审视特殊成员。
4. 普通类优先 Rule of Zero，把资源交给 `string`、`vector`、`unique_ptr` 等成员管理。
