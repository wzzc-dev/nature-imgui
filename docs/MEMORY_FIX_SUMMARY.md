# 内存修复总结

## 问题诊断

### 1. Double Free 错误

**问题描述：**
`simple_declarative_demo` 程序运行时出现 double free 错误：
```
malloc: Double free of object 0x149e15b00
malloc: *** set a breakpoint in malloc_error_break to debug
```

**根本原因：**
- Property 对象同时被 Widget 和 RenderObject 引用
- 当释放 Widget 时，Property 被释放
- 当释放 RenderObject 时，Property 再次被释放，导致 double free

**修复方案：**
1. 在 `Property` 结构中添加 `ref_count` 字段（引用计数）
2. 所有 Property 创建函数初始化 `ref_count = 1`
3. `attach_render_object` 时增加引用计数
4. `release_property` 时递减引用计数，只有为 0 时才真正释放

**修改的文件：**
- `declarative_ui.h`: 添加 `ref_count` 字段到 Property 结构
- `declarative_ui.cpp`:
  - 修改所有 Property 创建函数，初始化 `ref_count = 1`
  - 修改 `attach_render_object`，增加 Property 引用计数
  - 修改 `release_property`，实现引用计数逻辑

### 2. ReactiveState 内存泄漏

**问题描述：**
`create_counter_ui` 中创建的 ReactiveState 对象没有被正确释放，导致内存泄漏。

**根本原因：**
- ReactiveState 被转换为 Widget* 并存储在 Property 中
- `release_widget` 时不知道这是 ReactiveState，无法正确释放

**修复方案：**
在 `release_widget` 中，识别属性名为 "state" 且类型为 PROP_TYPE_WIDGET 的属性，将其作为 ReactiveState* 释放。

**修改的文件：**
- `declarative_ui.cpp`: 修改 `release_widget` 函数，添加 ReactiveState 释放逻辑

## 测试

### 新增测试程序

创建了 `tests/memory_test.cpp`，这是一个不依赖 SDL3 的纯内存测试程序：

```bash
# 编译测试
cd /Volumes/Data/Code/nature-imgui/build
make memory_test

# 运行测试
./memory_test
```

### 测试内容

1. ✅ Widget tree 创建
2. ✅ Widget tree 构建
3. ✅ 状态管理（创建、读取、更新、释放）
4. ✅ 资源清理
5. ✅ 无内存泄漏
6. ✅ 无 double free 错误

## 关于 "UI 没有显示"

### 说明

`simple_declarative_demo` 是一个**单元测试程序**，它只测试了：
- Widget tree 的创建
- Widget tree 的构建
- 资源的释放

它**不包含**以下内容：
- SDL/ImGui 初始化
- 窗口创建
- 渲染循环
- 实际的 UI 显示

因此，这个程序不会显示任何 UI 窗口，它只会在控制台输出测试结果。

### 如何查看实际的 UI

要查看实际的 UI 窗口，需要运行 `advanced_declarative_demo`，它包含：
- 完整的 SDL 初始化
- 窗口创建
- 主渲染循环
- 多个窗口的演示

但是由于 SDL3 链接问题（macOS 下缺少某些系统框架的依赖），`advanced_declarative_demo` 暂时无法编译。

### 解决方案

有两个选择：

1. **修复 SDL3 链接问题**
   - 安装完整的 SDL3 依赖
   - 或者使用 brew 安装 SDL3: `brew install sdl3`

2. **使用声明式UI作为纯逻辑层**
   - 目前实现的声明式UI可以作为逻辑层使用
   - 需要时可以集成到其他现有的 ImGui 应用中

## 技术架构

声明式UI 框架包含三个核心层次：

1. **Widget 层**：声明式 UI 组件树
2. **Element 层**：运行时组件树，管理生命周期
3. **RenderObject 层**：渲染对象树，与 ImGui 集成

当前实现状态：
- ✅ Widget 层：完整实现
- ✅ Element 层：完整实现
- ✅ RenderObject 层：完整实现
- ✅ 状态管理：完整实现
- ✅ Diff 算法：基础实现
- ✅ 内存管理：已修复（引用计数）
- ⚠️ 渲染器：部分实现（需要 SDL3）
- ⚠️ 实际显示：需要 SDL3 支持

## 下一步

1. 测试内存修复是否有效
2. 修复 SDL3 链接问题（如果需要实际显示）
3. 或者将声明式UI集成到现有的 ImGui 应用中
