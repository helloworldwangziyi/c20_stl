# 模板特化：主模板、偏特化、全特化

模板特化回答一个问题：**默认实现之外，能不能为某些类型换一份实现？**
答案是分三级的：主模板兜底、偏特化按"形状"匹配、全特化按"身份"命中。
`is_same` 和 `is_void` 分别用到了后两级，本文把家族成员一次讲清。

## 1. 主模板（primary template）：默认回答

特化的前提是先有"可被特化的东西"——主模板：

```cpp
template <typename T>
struct is_void : false_type {};   // 对一切 T 的默认回答：不是 void
```

主模板对任何实参都能实例化，是兜底答案。特化的作用就是：
**在特定条件下，用另一份实现替换它**。

## 2. 全特化（explicit/full specialization）：按身份命中

所有模板参数都定死，一份特化只服务一个具体类型组合：

```cpp
template <>
struct is_void<void> : true_type {};
// ^^^^^^^^ 模板参数列表为空：这是全特化的语法标志
```

要点：

- `template <>` 的**空列表**是全特化的身份标志——"没有参数可推了，
  全是死类型"。
- 写在尖括号里的是**具体类型**，不是参数名。
- 一份全特化只匹配一种实参组合。要覆盖 `void` 的四个 cv 变体，
  就得写四份全特化——这正是 `is_void` 练习的形状。

## 3. 偏特化（partial specialization）：按形状匹配

模板参数没定死，而是用**模式**表达"一类实参"：

```cpp
template <typename T, typename U>
struct is_same : false_type {};       // 主模板：默认 false

template <typename T>
struct is_same<T, T> : true_type {};  // 偏特化：两参同型时命中
```

`<T, T>` 是一个模式：编译器把实参代进去匹配，`(int, int)` 命中、
`(int, bool)` 不命中。偏特化能写的模式还有：

```cpp
template <typename T> struct foo<T*>;           // 任何指针
template <typename T> struct foo<T&>;           // 任何左值引用
template <typename T> struct foo<const T>;      // 任何 const 类型
template <typename T, std::size_t N>
struct foo<T[N]>;                               // 任何数组
```

注意偏特化的 `<>` 里**仍然有参数**——这是它和全特化的外观分界线：

| | 参数列表 | 尖括号内容 | 覆盖范围 |
|---|---|---|---|
| 全特化 | `template <>` 为空 | 具体类型，如 `<void>` | 一种组合 |
| 偏特化 | `template <typename T>` 仍有参数 | 模式，如 `<T, T>`、`<T*>` | 一类组合 |

## 4. 关键认知：模式匹配"形状"，匹配不了"身份"

这是理解 `is_void` 为什么只能全特化的钥匙。

偏特化的模式描述的是类型的**结构**（是指针吗？是引用吗？是 const
吗？是两参相同吗？），而 `void` 的判定靠的是**身份**——"你就是
`void` 这个类型"。没有任何模式能表达"只匹配 void 不匹配别的"：

- `const T` 模式？它会把 `const int` 也当成 void，错。
- 没有"void-ness"这种结构特征可抓。

所以特定类型的判定只能**枚举全特化**；形状的判定才轮得到偏特化。

## 5. 编译器的选择顺序

一个模板名后面跟实参时，编译器按这个优先级挑实现：

1. **全特化**：实参与某份全特化完全相同时，用它（最优先）；
2. **偏特化**：按"更特化者优先"（partial ordering）在偏特化里挑
   最匹配的一份；
3. **主模板**：都不匹配时兜底。

例：`is_same<int*, int*>` 走偏特化 `<T, T>`（T 推导为 `int*`）；
`is_same<int, bool>` 哪份偏特化都不匹配，落回主模板。

## 6. 函数模板的特化：只有全特化，没有偏特化

经典考点，提前避雷：**函数模板只允许全特化**：

```cpp
template <typename T>
void f(T x) { /* 通用 */ }

template <>
void f<void*>(void* p) { /* 只服务 void* */ }   // 合法：全特化
```

想对"一类类型"（所有指针）换实现？不允许偏特化，正确姿势是**重载**：

```cpp
template <typename T>
void f(T* p) { /* 所有指针走这里 */ }   // 重载，不是特化
```

类模板没有重载机制（同名类只能有一个），所以类模板用偏特化、
函数模板用重载——两条路解决同一个"按形状分流"的问题。

## 7. 常见误区

- **"偏特化是重载"**——错。重载是函数的行为；类模板只有特化，
  没有重载。
- **忘记 `template <>`**——写全特化时漏掉空列表，编译器会把它
  当成新模板报重定义。
- **以为一份特化能覆盖 cv 变体**——`const void` 是独立类型，
  全特化必须逐个枚举。
- **在类里乱特化**——成员函数模板特化规则更绕（不能局部特化），
  现阶段遇到先查资料再动手。

## 8. 回到 is_void

| | is_same | is_void |
|---|---|---|
| 主模板 | `: false_type` | `: false_type` |
| 特化手段 | 偏特化 `<T, T>` | 全特化 × 4（void 及三个 cv 变体） |
| 匹配依据 | 形状：两参同型 | 身份：就是 void |

语法模板照抄本文第 2 节，四份全特化 + 一行 `is_void_v`，
验收照旧用 `std::is_void_v` 当裁判。

## 9. 特化里的继承：复用与答案切换

主模板和特化都写作 `: 某某_type`，这里的继承承担两个职责：

**代码复用**。trait 要可用至少得提供 `::value`，最好还有 `type`、
`value_type`、bool 转换。不继承的话每个 trait 都要复制这套；而
`true_type`/`false_type` 本身就是 `integral_constant` 预装好的
整套答案，继承一行全部到手。

**答案切换**。主模板与特化给**不同的基类**，就是在两种答案间做选择：

```cpp
template <typename T>
struct is_void : false_type{};        // 默认：不是 void

template <>
struct is_void<void> : true_type{};   // 命中：是 void
```

实例化时编译器挑中哪份实现，`value` 就是哪份基类携带的值。
trait 的"判断"本质是继承关系的二选一。

`is_void<void>::value` 的查找链（名字查找沿继承向上爬）：

```
is_void<void>        自己没有 value
  └─ true_type       别名 → bool_constant<true>
       └─ integral_constant<bool, true>
            value = true    ← 命中
```

整条链没有 virtual、没有运行时代价，是纯编译期的实现复用。
`true_type`/`false_type` 就是为此存在的两个"预制答案"。
