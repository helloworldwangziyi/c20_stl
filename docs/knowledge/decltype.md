# `decltype` 的用法

`decltype` 用于在编译期查询表达式的类型，**不求值**，并且完整保留类型的引用和 cv 限定。

## 1. 基本用法

```cpp
int x = 0;
const int y = 1;
int& r = x;

decltype(x) a;      // int
decltype(y) b;      // const int —— 顶层 const 保留
decltype(r) c = x;  // int&      —— 引用保留

std::vector<decltype(x)> v;   // 任何需要类型的地方都能用
```

## 2. 核心规则：`decltype(x)` 和 `decltype((x))` 不一样

- **不加括号**：操作数是变量名或成员访问 → 返回该实体的**声明类型**。
- **加了括号**：操作数变成表达式 → 按表达式的**值类别**决定结果：

| 表达式类别 | `decltype(expr)` 结果 |
|---|---|
| 左值 (lvalue) | `T&` |
| 将亡值 (xvalue) | `T&&` |
| 纯右值 (prvalue) | `T` |

```cpp
int x = 0;
decltype(x)   a;      // int  —— 声明类型
decltype((x)) b = x;  // int& —— (x) 是左值表达式
```

函数调用也是同理：`decltype(f())` 按 `f()` 这个表达式的值类别推导（`f` 返回 `int&` 则结果是 `int&`，返回 `int` 则结果是 `int`）。注意这里只求类型，**不会真的调用 `f`**。

## 3. 与 `auto` 的区别

`auto` 会剥掉引用和顶层 const，`decltype` 一律保留：

```cpp
const int x = 0;
int& r = x;

auto a1 = x;        // int
decltype(x) b1 = x; // const int

auto a2 = r;        // int   —— auto 剥掉引用
decltype(r) b2 = r; // int&  —— decltype 保留引用
```

选择原则：想要「表达式的精确类型」用 `decltype`；想要「剥壳之后的值类型」用 `auto`。

## 4. `decltype(auto)`（C++14）

`decltype(auto)` 表示「用 `auto` 占位，但按 `decltype` 的规则推导」：

```cpp
int g = 0;
int& get() { return g; }

auto f1()           { return get(); }  // 返回 int  —— auto 剥引用
decltype(auto) f2() { return get(); }  // 返回 int& —— 完美保留

// 变量声明同理
decltype(auto) r = get();   // int&
```

典型用途：**转发函数的返回值**，保证引用语义不被 `auto` 剥掉。

对应的坑：

```cpp
decltype(auto) bad() { return (g); }  // 返回 int&!
// (g) 是左值表达式,decltype 给出引用;
// 如果 g 是局部变量,返回的就是悬垂引用
```

## 5. 实际应用场景

### 5.1 尾置返回类型（C++11）

```cpp
template <typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
}
```

C++11 时参数名在返回类型位置不可见，必须借助尾置返回类型。C++14 起直接写 `auto add(T a, U b)` 让编译器推导即可，但这种写法在需要精确控制返回类型时仍然有用。

### 5.2 模板元编程 / SFINAE

```cpp
template <typename T>
auto size(const T& t) -> decltype(t.size()) {  // T 没有 .size() 时此重载被 SFINAE 排除
    return t.size();
}
```

### 5.3 配合 `std::declval`

`std::declval<T>()` 返回 `T&&`，**不求值**，只能出现在 `decltype` 等不求值上下文中：

```cpp
template <typename T, typename U>
using SumT = decltype(std::declval<T>() + std::declval<U>());
// T、U 可能不可默认构造、不可拷贝,declval 让我们不必创建对象就能问出类型
```

## 6. 常见坑

- `decltype((x))` 多一层括号就可能多一层引用，左值表达式推出 `T&`。
- 返回类型选 `auto` 会把引用剥掉导致意外拷贝；选 `decltype(auto)` 则可能返回悬垂引用，二者要看场景。
- `decltype` 的操作数不求值：写 `decltype(f())` 不会调用 `f`，但这也意味着**副作用不会发生**：

```cpp
int i = 0;
decltype(i++) x;   // 合法,但 i 不会真的自增
```

- `std::declval` 放进求值上下文是未定义行为，它只能配合 `decltype` 等使用。

## 小结

| 写法 | 结果 |
|---|---|
| `decltype(x)` | `x` 的声明类型（保留引用和 cv） |
| `decltype((x))` | 按值类别：左值 `T&`、将亡值 `T&&`、纯右值 `T` |
| `auto` | 剥掉引用和顶层 const 的值类型 |
| `decltype(auto)` | 按 `decltype` 规则推导，保留引用 |
| `decltype(f())` | `f` 返回值的类型（若返回引用则保留），不求值 |
