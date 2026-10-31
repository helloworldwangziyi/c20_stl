# 项目路线图：通往常用容器

## 目标与毕业线

**目标**：手写 C++20 常用容器。

**毕业线**：`vector` + `list` + `unordered_map` 到手，即目标达成。
`map`/红黑树、`deque`、`string` 为附录选做，不设期限。

核心逻辑：容器不是起点，是地基上的建筑。直接开写 vector 会在内存管理
和类型萃取上反复卡住，所以前面几个阶段都是在给 vector 铺路。

预计周期：约两个半月到三个月（按每天 1-2 小时、每 trait 吃透的节奏）。

---

## 阶段一：type_traits 手写核心（约 2-3 周）

| 状态 | 内容 | 备注 |
|---|---|---|
| ✅ | `integral_constant` / `bool_constant` / `true_type` / `false_type` | 已完成 |
| ✅ | `is_same` | 已完成 |
| ✅ | `is_same_v` 变量模板 | 已完成，`_v` 惯例跑通 |
| ✅ | `is_void`、`is_null_pointer` | 含 cv 四组合特化及测试 |
| ✅ | `is_const`、`is_volatile` | 顶层/底层判定及测试 |
| ✅ | `remove_const` | 变换型 trait 首个，含完整测试矩阵 |
| ✅ | `add_const` | 单主模板，无需偏特化 |
| ✅ | `remove_volatile` | `remove_const` 的换字克隆 |
| ⬜ | `remove_reference`、`add_lvalue_reference`、`add_rvalue_reference` | vector 的拷贝/移动构造离不开 |
| ⬜ | `is_array`、`rank`、`extent` | |
| ⬜ | `conditional`、`enable_if` | |
| ⬜ | 类型清单手写 `is_integral`、`is_floating_point`、`is_arithmetic`、`is_signed` | 进阶一试，不碰内建（libstdc++ 同款做法） |

## 阶段二：内存与构造（约 1 周，容器前置件）

vector 的本质是"裸内存 + 在这块内存上构造对象"，先造工具：

- ⬜ `allocator`：`operator new/delete` 的封装（allocate/deallocate）
- ⬜ `construct_at` / `destroy_at`（placement new + 显式析构调用）
- ⬜ `uninitialized_copy` / `uninitialized_fill` / `uninitialized_move`
- ⬜ 测试：造一个**计数类型**（构造/析构次数、是否拷贝/移动），
  所有内存工具用它验证——这个测试工具写 vector 时要一直用

## 阶段三：vector（重头戏，约 2-3 周）

- ⬜ 内部结构：三根指针（first / last / end_of_storage）
- ⬜ 构造五件套：默认 / n 个值 / 迭代器区间 / 拷贝 / 移动
- ⬜ 访问：`operator[]`、`at`、`front`、`back`、`data`
- ⬜ 修改：`push_back`、`emplace_back`、`pop_back`、`clear`
- ⬜ 容量：`size`、`capacity`、`empty`、`reserve`、`resize`
- ⬜ 迭代器（先指针，后升级为类）
- ⬜ 关键概念（各值得一天）：RAII 五法则、扩容策略、**异常安全**
  （元素移动构造不 noexcept 时退化为拷贝——vector 最经典的一课）
- ⬜ 验收：每个操作与 `std::vector` 行为逐条对照，`ctest` 全绿

## 阶段四：list（约 1-2 周）

- ⬜ 节点结构 `_List_node<T>` + 哨兵头节点
- ⬜ **真·迭代器类**：重载 `operator++/--/*`，体会与 vector
  "裸指针迭代器"的本质区别
- ⬜ `insert` / `erase` / `push_front` / `push_back`
- ⬜ 进阶选做：`splice`、`sort`
- ⬜ 验收：对照 `std::list`

## 阶段五：一个关联容器（约 2-3 周，毕业作品）

- ⬜ 首选 `unordered_map`：桶数组 + 开链 + `rehash` + `load_factor`
- ⬜ 选做（硬核毕业设计）：`map` + 红黑树

---

## 贯穿规则

1. **每完成一个 trait / 一个操作就提交一次**——节奏感比速度重要；
2. **真 STL 永远当裁判**：行为对照 `std::vector`，类型检查用
   `std::is_same_v`；
3. C++20 的写法：接口约束用 concepts/requires，能用 `if constexpr`
   就不用 tag dispatch；
4. 每周日更新本文档的勾选状态，可视化进度。

---

## 进度日志

| 日期 | 里程碑 |
|---|---|
| 2026-10-26 | 项目初始化 |
| 2026-10-27 | `integral_constant` 及测试 |
| 2026-10-28 | `bool_constant`/`true_type`/`false_type`、`is_same`；路线图定稿 |
| 2026-10-29 | `is_same_v` 变量模板；补 `inline` 知识点文档 |
| 2026-10-30 | `is_void`/`is_null_pointer` 补 cv 特化及测试；`is_const` 偏特化（待补测试） |
| 2026-10-31 | `is_const`/`is_volatile` 毕业；顶层/底层 const 文档（钥匙与封条） |
| 2026-11-01 | `remove_const` 首个变换型 trait（实现 + 测试矩阵）；确立 AGENTS.md 教学约定 |
| 2026-11-02 | `add_const`（单主模板）；cv 家族混测边角补齐 |
| 2026-11-03 | `remove_volatile` 毕业；`typename` 依赖名原理吃透 |
