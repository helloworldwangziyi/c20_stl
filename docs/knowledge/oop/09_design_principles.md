# 第 9 章：面向对象设计原则

设计原则的价值是帮助我们控制变化与耦合，不是把类拆得越多越好；每条原则都应落到
对象职责、接口契约和替换关系上。

## 1. 高内聚、低耦合

**高内聚**指一个类中的数据和操作围绕同一个清晰职责协作。例如 `bank_account` 处理余额
规则，就不应同时负责发送邮件和拼接网页。

**低耦合**指一个类尽量少依赖其他类的内部细节。只依赖公开接口，比读取对方私有结构、
知道对方具体实现步骤更稳定。

可以用两个问题检查：

- 修改这个类时，是否总要顺便修改许多无关类？
- 给类写一句职责说明时，是否充满“并且”“顺便”“还负责”？

如果答案明显，通常需要重新划分职责，但也不要把每个小操作都机械拆成一个类。

## 2. 组合优先于继承

组合把另一个对象作为成员，通过公开接口复用能力：

```cpp
class engine {
public:
    void start() { ++starts; }
    int starts = 0;
};

class car {
public:
    void start() { engine_.start(); }
    int engine_starts() const { return engine_.starts; }

private:
    engine engine_;
};
```

`car` 使用 `engine`，不需要承诺“汽车可以在所有场景中替代发动机”。组合通常让依赖更
局部，也避免暴露不需要的基类接口。

继承仍有价值：当对象确实需要共享基类契约、支持替换或多态时，公有继承才是合适工具。
“优先组合”不是“禁止继承”。

## 3. 里氏替换：派生类不能破坏基类承诺

里氏替换原则要求：使用基类接口的代码，换成派生类对象后仍应得到符合契约的行为。

经典的 `rectangle` / `square` 问题说明，数学上的“是一种”不一定适合可变对象接口：
如果基类允许分别设置宽和高，正方形却要求两者永远相等，派生类就无法自然满足所有
矩形操作背后的预期。

检查派生类时可以问：

- 是否加强了基类没有要求的前置条件？
- 是否削弱了基类承诺的结果？
- 是否让基类合法操作在派生类上突然失效？
- 是否破坏了调用者依赖的对象不变量？

继承关系应依据行为契约，而不是只依据名词分类。

## 4. 接口隔离：不要强迫实现者提供无关能力

一个臃肿接口可能要求所有实现者都提供读、写、格式化、网络发送等操作，即使某些实现
只支持读取。更合理的方式是按能力拆分：

```cpp
class reader {
public:
    virtual ~reader() = default;
    virtual int read() = 0;
};

class writer {
public:
    virtual ~writer() = default;
    virtual void write(int value) = 0;
};
```

只读类型实现 `reader`，可写类型实现 `writer`，调用者只依赖自己真正需要的能力。
接口隔离不等于接口越小越好，而是避免把互不相关的职责硬捆在一起。

## 5. 依赖抽象，而不是具体实现

如果日志类直接创建某种文件输出对象，替换为内存输出或网络输出就会修改日志类。
可以让它依赖一个抽象输出接口，并由外部注入实现：

```cpp
class sink {
public:
    virtual ~sink() = default;
    virtual void write(std::string_view text) = 0;
};

class logger {
public:
    explicit logger(sink& output) : output_(output) {}
    void log(std::string_view text) { output_.write(text); }

private:
    sink& output_;
};
```

高层 `logger` 和低层输出实现都依赖 `sink` 抽象。调用者选择具体实现，日志逻辑不必改变。
这里 `logger` 只借用 `sink`，不拥有它，因此调用者必须保证输出对象活得比 `logger` 久。

## 6. 值类型、实体类型、资源管理类型

不同对象需要不同的复制和身份语义：

| 类型 | 主要特征 | 常见设计 |
|---|---|---|
| 值类型 | 比较内容，副本相互独立 | 支持拷贝，例如 `point`、`string` |
| 实体类型 | 身份比当前属性更重要 | 谨慎拷贝，常用稳定 ID |
| 资源管理类型 | 负责独占或共享资源所有权 | 按所有权禁止或设计拷贝、支持移动 |

例如账户可能具有不可重复的身份，简单复制所有字段会制造“两个同身份账户”；坐标点则
通常可以安全复制。设计特殊成员函数前，先判断对象属于哪种语义。

## 7. SOLID 作为检查表

| 原则 | 实际要检查的问题 |
|---|---|
| 单一职责 SRP | 类的变化原因是否清晰、集中？ |
| 开闭原则 OCP | 新需求能否通过扩展接口实现，而非反复修改稳定逻辑？ |
| 里氏替换 LSP | 派生类是否始终遵守基类契约？ |
| 接口隔离 ISP | 调用者是否被迫依赖不需要的能力？ |
| 依赖倒置 DIP | 高层逻辑是否依赖抽象，而非具体低层实现？ |

这些原则相互关联，但不是评分表。为一个不会变化的小程序铺满抽象接口，同样会增加
复杂度和阅读成本。

## 8. 可编译示例：组合与依赖抽象

```cpp
#include <cassert>
#include <string>
#include <string_view>

class sink {
public:
    virtual ~sink() = default;
    virtual void write(std::string_view text) = 0;
};

class string_sink final : public sink {
public:
    void write(std::string_view text) override { data += text; }
    std::string data;
};

class logger {
public:
    explicit logger(sink& output) : output_(output) {}
    void log(std::string_view text) { output_.write(text); }

private:
    sink& output_;
};

int main()
{
    string_sink output;
    logger log{output};
    log.log("ready");

    assert(output.data == "ready");
}
```

`logger` 不知道具体输出类型，测试因此可以使用 `string_sink` 观察结果，而无需创建文件。

## 9. 本章要点

1. 高内聚让职责集中，低耦合让修改影响保持局部。
2. 组合复用对象能力，继承承诺可替代关系，二者不能只看代码多少来选择。
3. 接口应围绕真实能力划分，高层逻辑通过抽象依赖可替换实现。
4. 原则服务于变化和契约，不应成为过度设计的理由。
