# 第 1 章：类与对象基础

类规定对象保存什么数据、允许什么操作；对象则是按照这些规定创建出来的具体实体。
面向对象的第一步不是继承，而是把数据和维护数据规则的操作放在一起。

## 1. 类与对象：图纸和实例

可以把类理解为图纸，把对象理解为按图纸造出的房子。图纸规定结构，每座房子却有
自己的状态和地址。

```cpp
class bank_account {
public:
    void deposit(int amount)
    {
        if (amount <= 0) {
            throw std::invalid_argument("deposit must be positive");
        }
        balance_ += amount;
    }

    int balance() const { return balance_; }

private:
    int balance_ = 0;
};
```

`bank_account` 是类，下面代码创建了三个独立对象：

```cpp
bank_account a;
bank_account b;
bank_account c;
```

`a`、`b`、`c` 各有自己的 `balance_`。成员函数逻辑上属于类，不需要在每个对象里
各存一份函数代码。

## 2. 数据成员与成员函数

**数据成员**描述对象的状态，例如账户余额、字符串长度、vector 的三根指针。

**成员函数**描述对象支持的操作，例如存款、清空字符串、向 vector 添加元素。
在成员函数中直接写 `balance_`，访问的是当前调用对象的成员：

```cpp
a.deposit(100);   // 修改 a.balance_
b.deposit(50);    // 修改 b.balance_
```

类把“数据”和“维护这些数据的操作”组织成一个整体，这叫作封装。

## 3. 对象、作用域与生命周期

**作用域**是名字在代码中可见的范围；**生命周期**是对象实际存在的时间。二者相关，
但不是同一个概念。

```cpp
#include <memory>

void f()
{
    bank_account a;                 // 离开 f 时自动销毁

    auto p = std::make_unique<int>(42);
}                                   // unique_ptr 销毁，同时释放所管理的 int
```

常见生命周期形式：

| 对象 | 通常何时销毁 |
|---|---|
| 局部自动对象 | 离开其作用域时 |
| 动态对象 | 由对应的所有权机制释放，例如 `unique_ptr` |
| 静态存储期对象 | 程序结束阶段 |
| 类的数据成员 | 随所属对象一起销毁 |

不要把局部对象的引用或指针交给比对象活得更久的使用者，否则会形成悬垂引用或指针。

## 4. public、protected、private

访问说明符限制的是**名字能否从某个上下文访问**，不是对象是否能看到自己的数据。

| 访问级别 | 类的成员函数 | 派生类 | 类外通过对象 |
|---|---|---|---|
| `public` | 可以 | 通常可以 | 可以 |
| `protected` | 可以 | 可以 | 不可以 |
| `private` | 可以 | 不可以直接访问 | 不可以 |

同一个类的不同对象可以访问彼此的私有成员，因为访问控制按类生效，不是按对象生效：

```cpp
class number {
public:
    bool same_as(const number& other) const
    {
        return value_ == other.value_;   // 合法：同类成员访问
    }

private:
    int value_ = 0;
};
```

`protected` 与继承的关系留到第 6 章；初学阶段优先使用 `public` 接口和 `private`
数据。

## 5. class 与 struct

两者都能有构造函数、成员函数、继承和模板，语言能力的区别只有默认访问规则：

- `class` 的成员默认是 `private`，默认继承也是私有继承。
- `struct` 的成员默认是 `public`，默认继承也是公有继承。

```cpp
class a { int value; };   // private
struct b { int value; };  // public
```

常见约定：`struct` 用于以公开数据为主的简单类型，`class` 用于需要维护不变量的
封装类型。这只是风格约定，不是语言限制。

## 6. 封装真正保护的是对象不变量

**对象不变量**是对象在有效状态下必须始终满足的条件。例如：

- 账户余额不能通过非法操作被破坏。
- vector 的大小不能超过容量。
- 迭代器的指针必须指向所属容器的合法位置。

把 `balance_` 设为私有，不是为了“藏数据”，而是为了阻止外部绕过检查直接写入：

```cpp
// a.balance_ = -100;   // 不允许绕开 deposit 的规则
```

公有成员函数是对象对外的**接口**，私有数据与辅助函数是**实现**。使用者只依赖接口，
实现者才有修改内部结构的自由。

## 7. 可编译示例

```cpp
#include <cassert>
#include <stdexcept>

class bank_account {
public:
    void deposit(int amount)
    {
        if (amount <= 0) {
            throw std::invalid_argument("deposit must be positive");
        }
        balance_ += amount;
    }

    int balance() const { return balance_; }

private:
    int balance_ = 0;
};

int main()
{
    bank_account a;
    bank_account b;

    a.deposit(100);
    a.deposit(50);
    b.deposit(20);

    assert(a.balance() == 150);
    assert(b.balance() == 20);
}
```

两个对象互不影响，外部也只能通过公开操作改变状态，这就是最基础的封装。

## 8. 本章要点

1. 类定义结构和行为，对象保存各自的状态。
2. 作用域描述名字可见范围，生命周期描述对象存在时间。
3. 访问控制按类生效，不是按对象生效。
4. 封装的目的不是隐藏语法细节，而是保护对象不变量、稳定公开接口。
