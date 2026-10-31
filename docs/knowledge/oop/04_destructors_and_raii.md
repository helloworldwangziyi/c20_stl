# 第 4 章：析构函数与 RAII

析构函数负责对象生命结束时的清理；RAII 则把资源释放绑定到对象销毁上，让正常返回、
提前返回和异常离开都能走同一条可靠路径。

## 1. 析构函数何时执行

局部对象离开作用域时，析构函数自动执行：

```cpp
struct scoped_flag {
    bool& destroyed;
    ~scoped_flag() { destroyed = true; }
};

void f()
{
    bool destroyed = false;
    {
        scoped_flag flag{destroyed};
        // destroyed 仍为 false
    }
    // flag 已析构，destroyed 为 true
}
```

同一作用域中的局部对象按构造完成的相反顺序析构。类的成员也按构造的相反顺序销毁：
先执行当前类析构函数体，再销毁成员和基类。

动态对象不会因裸指针离开作用域而自动销毁，因此现代 C++ 优先用 `unique_ptr` 等所有权
类型管理，而不是到处手写 `new` / `delete`。

## 2. 析构不是“可有可无的回收站”

普通数据成员由编译器自动处理，只有需要特殊清理或观察生命周期的类型才通常要写
析构函数：

```cpp
class file {
public:
    ~file();
};
```

vector 析构时要销毁元素并释放存储，文件包装类析构时要关闭文件，锁包装类析构时要
解锁。它们共同的特征是：**对象拥有某种必须归还的资源**。

## 3. 资源不只是内存

RAII 中的资源包括：

- 动态内存。
- 文件和操作系统句柄。
- 互斥锁。
- 网络连接。
- 需要成对调用获取、释放接口的其他能力。

RAII 的核心是：构造函数取得资源并建立所有权，析构函数释放资源，对象复制或移动的
规则与所有权语义一致。

## 4. 用 RAII 包装文件

```cpp
#include <cstdio>
#include <stdexcept>

class file {
public:
    explicit file(const char* path)
        : handle_(std::fopen(path, "rb"))
    {
        if (handle_ == nullptr) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~file()
    {
        if (handle_ != nullptr) {
            std::fclose(handle_);
        }
    }

    file(const file&) = delete;
    file& operator=(const file&) = delete;

private:
    std::FILE* handle_ = nullptr;
};
```

只要 `file` 对象正常析构，就不必在每个返回点单独调用 `fclose`。这里禁止拷贝，是为了
避免两个对象误以为各自都能关闭同一个句柄；移动语义可在第 5 章学习后补充。

## 5. 优先复用现成的 RAII 类型

很多资源不必重新包装：

```cpp
#include <memory>
#include <mutex>
#include <vector>

void f(std::mutex& mutex)
{
    auto data = std::make_unique<int>(42);
    std::vector<int> values{1, 2, 3};
    std::lock_guard lock(mutex);
    // 离开作用域时依次解锁、销毁 vector、释放动态内存
}
```

`unique_ptr`、`vector`、`string`、`lock_guard` 已经把资源管理写好了。普通类直接使用这些
成员，往往就不需要自定义析构函数，这就是第 5 章的 Rule of Zero。

## 6. 为什么析构函数不应抛异常

析构函数通常隐式是 `noexcept`。如果对象已经因为异常而进入栈展开，此时析构函数再抛出
异常，程序通常会直接 `std::terminate`。

因此析构函数应尽量做不失败的清理。确实可能失败的操作，应提供显式的 `close()`、
`commit()` 等接口让调用者处理错误，析构函数负责最后的兜底清理。

## 7. 构造函数抛异常时会发生什么

构造函数没有完成时，当前对象还不算完整构造成功，因此**不会调用当前类自己的析构
函数**；但已经构造完成的成员和基类会被销毁。

这说明资源最好直接交给 RAII 成员管理。如果构造函数先取得多个裸资源，中途才抛异常，
当前类析构函数不会替它清理，容易发生泄漏。

## 8. 可编译示例

```cpp
#include <cassert>
#include <cstdio>
#include <stdexcept>

struct scoped_flag {
    bool& destroyed;
    ~scoped_flag() { destroyed = true; }
};

class file {
public:
    explicit file(const char* path) : handle_(std::fopen(path, "rb"))
    {
        if (handle_ == nullptr) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~file()
    {
        if (handle_ != nullptr) {
            std::fclose(handle_);
        }
    }

    file(const file&) = delete;
    file& operator=(const file&) = delete;

private:
    std::FILE* handle_ = nullptr;
};

int main()
{
    bool destroyed = false;
    {
        scoped_flag flag{destroyed};
        assert(!destroyed);
    }
    assert(destroyed);
}
```

示例没有实际打开文件，但验证了析构函数随作用域结束自动执行；`file` 则展示同一原则
如何用于真实资源。

## 9. 本章要点

1. 析构函数在对象生命周期结束时执行，局部对象无需手动调用。
2. RAII 用对象生命周期管理资源，让异常路径也能自动清理。
3. 析构函数不应抛异常，可能失败的收尾操作应另设显式接口。
4. 优先组合现成 RAII 成员，而不是让每个类都直接管理裸资源。
