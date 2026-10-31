# 项目路线图：通往 MyTinySTL 规模

## 目标与毕业线

**目标**：手写 C++20 mini-STL，规模对齐 [Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)
（11 容器 + 完整算法库），但全程 C++20 风格（concepts/requires/`if constexpr`）。

**毕业线**：11 容器（vector、list、deque、stack、queue、priority_queue、map、set、
unordered_map、unordered_set、basic_string）+ 核心算法库（排序/查找/堆/集合/数值），
每个组件都有 std 对照测试。

**策略**：
- MyTinySTL 当**目录**不当代码——逐组件对齐清单，每行自己写；
- type_traits 深度已反超目标（对方是 906B 迷你版），保持现有手写深度；
- 测试纪律不降：真 STL 永远当裁判。

预计周期：6-9 个月（每天 1-2 小时）。

---

## 阶段一：type_traits 手写核心（收官倒计时）

| 状态 | 内容 | 备注 |
|---|---|---|
| ✅ | `integral_constant` / `bool_constant` / `true_type` / `false_type` | |
| ✅ | `is_same`（+ `_v`） | |
| ✅ | `is_void`、`is_null_pointer` | 含 cv 四组合特化及测试 |
| ✅ | `is_const`、`is_volatile` | 顶层/底层判定及测试 |
| ✅ | `remove_const` | 变换型 trait 首个，含完整测试矩阵 |
| ✅ | `add_const` | 单主模板，无需偏特化 |
| ✅ | `remove_volatile` | `remove_const` 的换字克隆 |
| ✅ | `remove_reference` | 双偏特化（`T&`/`T&&`） |
| ✅ | `add_lvalue_reference`、`add_rvalue_reference` | 单主模板，引用折叠兜底 |
| ⬜ | `is_array`、`rank`、`extent` | 偏特化 `T[N]` / `T[]` 登场 |
| ⬜ | `conditional`、`enable_if` | 元编程两个开关 |
| ⬜ | 类型清单手写 `is_integral`、`is_floating_point`、`is_arithmetic`、`is_signed` | 进阶一试，不碰内建 |

## 阶段二：内存与构造（容器前置件）

vector 的本质是"裸内存 + 在这块内存上构造对象"，先造工具：

- ⬜ `allocator`：`operator new/delete` 的封装（allocate/deallocate）
- ⬜ `construct_at` / `destroy_at`（placement new + 显式析构调用）
- ⬜ `uninitialized_copy` / `uninitialized_fill` / `uninitialized_move`
- ⬜ 计数类型测试工具（构造/析构次数、拷贝/移动追踪）——写 vector 时一直用
- ⬜ 进阶选做：内存池（MyTinySTL `alloc.h` 对应物，自由链表按 size 分级）

## 阶段三：iterator + util（基础设施补齐）

- ⬜ `util`：`pair`、`move`、`forward`、`swap`（MyTinySTL `util.h` 对应物）
- ⬜ 迭代器标签体系（`input_iterator_tag` → `random_access_iterator_tag`）
- ⬜ `iterator_traits` 简化版 + `reverse_iterator`
- ⬜ `distance` / `advance` 基础版

## 阶段四：vector + algobase（第一个容器）

- ⬜ vector：三根指针结构、构造五件套、访问、修改、容量、迭代器
- ⬜ 关键概念：RAII 五法则、扩容策略、异常安全（移动不 noexcept 时退拷贝）
- ⬜ `algobase`：`copy` / `fill` / `max` / `min` / `lexicographical_compare`
- ⬜ 验收：每个操作与 `std::vector` 逐条对照，`ctest` 全绿

## 阶段五：list + deque + 容器适配器

- ⬜ list：节点结构 + 哨兵、真·迭代器类、`insert`/`erase`/`splice`（进阶）
- ⬜ **deque**：分段连续结构（MyTinySTL 版 35KB，隐藏硬骨头，留足时间）
- ⬜ stack / queue / priority_queue（容器适配器，基于 deque/vector）
- ⬜ `heap_algo`：`push_heap` / `pop_heap` / `make_heap` / `sort_heap`

## 阶段六：hashtable → unordered_map / unordered_set

- ⬜ 开链哈希表：桶数组 + 节点 + `rehash` + `load_factor`
- ⬜ `unordered_map` / `unordered_set` 外壳（哈希 + 迭代器包装）
- ⬜ `functional`：常用哈希函数对象

## 阶段七：rb_tree → map / set（全场最大单体）

- ⬜ 红黑树：节点、插入（变色+旋转）、删除（双倍黑处理）、迭代器
- ⬜ `map` / `set` / `multimap` / `multiset`（选做后两个）
- ⬜ `set_algo`：集合运算（`set_union` / `set_intersection` 等）

## 阶段八：basic_string

- ⬜ 动态字符串：SSO（短字符串优化）可选、COW 不做
- ⬜ 与 `std::string` 行为对照测试

## 阶段九：算法库收尾

- ⬜ `algo.h` 对应物：排序家族（quicksort/merge/intro 简化）、二分查找家族、`find` 家族
- ⬜ `numeric`：`accumulate` / `iota` / 相邻差分
- 原则：不攒到最后——排序/查找在 vector 毕业后就分批落地

---

## 贯穿规则

1. **每完成一个 trait / 一个操作就提交一次**——节奏感比速度重要；
2. **真 STL 永远当裁判**：行为对照 `std::vector`，类型检查用 `std::is_same_v`；
3. C++20 的写法：接口约束用 concepts/requires，能用 `if constexpr` 就不用 tag dispatch；
4. 每周日更新本文档勾选状态，可视化进度；
5. MyTinySTL 是目录不是答案：任何组件先出任务书、自己实现、测试驱动。

---

## 规模对照（2026-11-04 重定标）

| | 目标（MyTinySTL） | 当前 |
|---|---|---|
| 容器 | 11 个 | 0 |
| 数据结构 | rb_tree + hashtable | 0 |
| 算法 | 5 个头文件（algo 80KB） | 0 |
| 基础设施 | 迷你 traits | traits 深度已反超 ✅ |
| 测试 | 功能测试框架 | static_assert 裁判体系（更高标准）✅ |

---

## 进度日志

| 日期 | 里程碑 |
|---|---|
| 2026-10-26 | 项目初始化 |
| 2026-10-27 | `integral_constant` 及测试 |
| 2026-10-28 | `bool_constant`/`true_type`/`false_type`、`is_same`；路线图定稿 |
| 2026-10-29 | `is_same_v` 变量模板；补 `inline` 知识点文档 |
| 2026-10-30 | `is_void`/`is_null_pointer` 补 cv 特化及测试；`is_const` 偏特化 |
| 2026-10-31 | `is_const`/`is_volatile` 毕业；顶层/底层 const 文档（钥匙与封条） |
| 2026-11-01 | `remove_const` 首个变换型 trait；确立 AGENTS.md 教学约定 |
| 2026-11-02 | `add_const`（单主模板）；cv 家族混测边角补齐 |
| 2026-11-03 | `remove_volatile` 毕业；`typename` 依赖名原理吃透 |
| 2026-11-04 | `remove_reference` 毕业；引用/typename 笔记；OOP 系列文档；**目标重定标 MyTinySTL，路线图重排** |
| 2026-11-04 | `add_lvalue_reference`/`add_rvalue_reference` 毕业（引用折叠） |
