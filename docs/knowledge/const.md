# `const int*` 与 `int* const`：顶层 const 与底层 const

一句话区别：**const 锁的东西不同**——`int *const` 锁指针自己（不能改指向），
`const int*` 锁指向的对象（不能改值）。指针变量有两层：箭头本身（存的地址）
和箭头指向的格子（对象的内存），const 可以只锁其一。

## 1. 读法规则：从右往左，const 绑左边

声明从右往左念；`const` 默认绑定它**左边**的东西，左边没东西时才绑右边：

```cpp
int *const p;       // p 是 const 的指针，指向 int —— const 左边是 *,锁指针
const int *q;       // q 是 指针，指向 const int —— const 左边没东西，绑右边，锁对象
const int *const r; // 两层都锁：const 指针指向 const int
```

## 2. 行为对照

```cpp
int x = 1, y = 2;

int *const p = &x;
//  p ═══════► x        箭头焊死
p = &y;     // 错:assignment of read-only variable 'p'
*p = 42;    // 对:x 本身没锁

const int *q = &x;
//  q ───────► x🔒      对象上锁
q = &y;     // 对:箭头自由
*q = 42;    // 错:assignment of read-only location '*q'
```

| 写法 | 能改指向？ | 能改指向的值？ | const 锁的是 |
|---|---|---|---|
| `int*` | ✅ | ✅ | 无 |
| `int *const` | ❌ | ✅ | 指针自身（顶层） |
| `const int*` | ✅ | ❌ | 指向的对象（底层） |
| `const int *const` | ❌ | ❌ | 两层 |

## 3. 顶层 const 在类型系统里的特殊待遇

顶层 const 描述的是"变量自己"，因此：

- **按值传参/按值返回会剥掉它**：`void f(int)` 和 `void f(const int)`
  是同一个函数（参数顶层的 const 不参与函数类型）；而 `const int*`
  的底层 const 参与类型，`f(int*)` 与 `f(const int*)` 是两个重载。
- **模板实参按值传递时同样剥掉**：`is_const_v<int* const>` 为 true，
  但把 `int* const` 传给 `template<class T> void g(T)` 后，`T` 将是
  `int*`——顶层 const 没了，底层 const 永远保留。

## 4. 与 trait 的关系：`is_const` 只报顶层

`c20_stl::is_const` 检查的是类型**最外层**（对象自身）的 const：

```cpp
static_assert( c20_stl::is_const_v<int *const>);   // true :对象自身是 const 指针
static_assert(!c20_stl::is_const_v<const int *>);  // false:对象是普通指针,const 在底层
```

底层 const 属于"指向类型"的一部分，不影响指针对象自身的可赋值性，
所以 `is_const` 不算数。要探测底层 const，得对"去掉一层指针之后的
类型"再调 `is_const`——这正是 `remove_pointer` + `is_const` 组合的
经典用途，也是后续 trait 的积木玩法。


---

## 5. 为什么语言要区分这两种 const：钥匙与封条

**指针是一把钥匙，指向的对象是房间，const 就是焊条。**

- `int *const p`：钥匙焊死在钥匙环上——不能拿它开别的门（改指向 ❌），
  但它能开的那间房随便折腾（改值 ✅）。锁的是**钥匙本身**。
- `const int *q`：房间贴了"展品勿动"——钥匙随便换门（改指向 ✅），
  进门后什么都不能碰（改值 ❌）。锁的是**房间**。

### 顶层 const = 私事，不传给别人

给自家钥匙焊环纯属个人防丢习惯。别人配一把同款钥匙（拷贝），拿到的
就是普通钥匙——焊法传不出去。所以语言干脆不记录它：

- 函数类型里没有顶层 const：`void f(int*)` 与 `void f(int* const)`
  是同一个函数（编译报 redefinition）；
- 模板按值拷贝/推导时顶层 cv 被自动剥掉。

```cpp
template<class T> void what(T) { std::cout << std::is_const_v<T> << '\n'; }
const int x = 1;
int* const p = nullptr;
what(x);              // T = int   → 0（顶层被剥）
what(p);              // T = int*  → 0（顶层被剥）
what<const int>(x);   // 显式指定，T 原样保留 → 1
```

### 底层 const = 对象的公约，谁拿钥匙谁遵守

"这个对象不许改"是对象的公共属性，因此它写进类型身份：

- `void g(int*)` 与 `void g(const int*)` 是**两个真函数**（合法重载）；
- 转换单向：`int*` → `const int*` 随意（贴封条永远安全），
  反向非法（撕封条 = 毁约）。

撕封条为什么必须禁止：

```cpp
const int x = 42;   // 博物馆签了"永不改建"合同
const int* p = &x;  // 你拿的是参观证
int* q = p;         // 若允许：参观证换施工证
*q = 0;             // 拆博物馆，合同成废纸 → 编译器直接禁止这步
// error: invalid conversion from 'const int*' to 'int*'
```

底层 const 就是 const 正确性的守门员：保证"承诺只读"的对象不会被
任何路径偷偷改写。

## 6. 一句话总结

**顶层 const 管钥匙的主人（私事，随拷贝消失，不入类型身份）；
底层 const 管房间的合约（公事，跟着类型走，写入类型身份）。**

## 7. 对 trait 的意义

类型是一棵树：`int* const` 的 const 焊在**大门口**，`const int*` 的
const 焊在**屋里**。`is_const` 是只看大门口的保安：

```cpp
static_assert( c20_stl::is_const_v<int *const>);   // 封条在门口 → true
static_assert(!c20_stl::is_const_v<const int *>);  // 封条在屋里 → 管不着
```

而 `remove_const_t<const int*>` 必须**原样返回**：若连屋里的封条也撕，
等于替陌生人撬锁——const 安全模型当场破产。这对测试守护的不只是
实现正确，是整个类型系统的安全承诺。
