# 声明式UI架构设计文档

## 概述

本文档描述了基于Dear ImGui的声明式UI框架的设计和实现。该框架提供了类似SwiftUI和Flutter的开发体验，同时保持与现有即时模式UI系统的兼容性。

## 设计目标

1. **声明式语法**: 通过描述UI状态而非命令式操作来构建UI
2. **响应式数据绑定**: 状态变化自动触发UI更新
3. **类型安全**: 强类型系统，编译时错误检查
4. **内存安全**: 自动内存管理，避免内存泄漏
5. **高性能**: 智能Diff算法，最小化不必要的渲染
6. **向后兼容**: 与现有Dear ImGui API完全兼容

## 核心架构

### 1. 三层架构

```
┌─────────────────────────────────────────────────────────┐
│                    用户代码层                              │
│  (声明式Widget创建、状态管理、事件处理)                     │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                    框架核心层                              │
│  (Widget、Element、RenderObject、状态管理、Diff算法)       │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                   渲染桥接层                              │
│  (Element到Dear ImGui的渲染、事件转发、性能优化)          │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                   Dear ImGui层                            │
│  (即时模式GUI、WebGPU渲染、SDL3窗口管理)                  │
└─────────────────────────────────────────────────────────┘
```

### 2. 核心概念

#### Widget（组件）
- **不可变**: Widget是纯描述性的，创建后不可修改
- **轻量级**: 只包含配置信息，不保存状态
- **可组合**: 通过组合简单Widget构建复杂UI

```cpp
Widget* create_button_widget(const char* label, EventHandlerFn callback);
```

#### Element（运行时实例）
- **可变**: Element是Widget的运行时实例，可以更新
- **状态管理**: 管理Widget的生命周期和状态
- **父子关系**: 维护组件树结构

```cpp
StatefulElement* create_element(Widget* widget, StatefulElement* parent);
```

#### RenderObject（渲染对象）
- **高性能**: 优化后的渲染对象，负责实际绘制
- **缓存布局**: 缓存布局计算结果
- **按需更新**: 只在需要时重新计算布局

```cpp
RenderObject* create_render_object(WidgetType type);
```

### 3. 响应式状态系统

#### 状态创建
```cpp
ReactiveState* state = create_state_bool(false);
```

#### 状态读取
```cpp
bool value = get_state_bool(state);
```

#### 状态更新
```cpp
set_state_bool(state, true);  // 自动触发监听器
```

#### 状态监听
```cpp
add_state_listener(state, listener_callback, user_data);
```

### 4. Diff算法

#### 算法原理
1. **类型检查**: 比较Widget类型
2. **Key匹配**: 使用唯一Key识别Widget
3. **属性比较**: 比较Widget属性差异
4. **子树Diff**: 递归比较子组件

#### Diff结果
```cpp
typedef struct {
    Widget** updated_widgets;  // 需要更新的Widgets
    Widget** removed_widgets;  // 需要移除的Widgets
    Widget** added_widgets;    // 需要添加的Widgets
} DiffResult;
```

### 5. 渲染流程

#### 帧渲染流程
```
1. begin_frame()
   ↓
2. 检查WidgetTree是否需要重建
   ↓
3. 如果需要，执行Diff并应用更新
   ↓
4. layout_widget_tree()
   ↓
5. paint_widget_tree()
   ↓
6. imgui_render()
   ↓
7. end_frame()
```

## 类型系统

### Widget类型
```cpp
typedef enum {
    WIDGET_TYPE_TEXT,      // 文本
    WIDGET_TYPE_BUTTON,    // 按钮
    WIDGET_TYPE_CHECKBOX,  // 复选框
    WIDGET_TYPE_SLIDER,    // 滑块
    WIDGET_TYPE_CONTAINER, // 容器
    WIDGET_TYPE_WINDOW,    // 窗口
} WidgetType;
```

### 属性类型
```cpp
typedef enum {
    PROP_TYPE_STRING,        // 字符串
    PROP_TYPE_FLOAT,         // 浮点数
    PROP_TYPE_BOOL,          // 布尔值
    PROP_TYPE_INT,           // 整数
    PROP_TYPE_CALLBACK,      // 回调函数
    PROP_TYPE_WIDGET,        // 子Widget
    PROP_TYPE_WIDGET_ARRAY,  // Widget数组
    PROP_TYPE_COLOR,         // 颜色
} PropertyType;
```

### 状态类型
```cpp
typedef enum {
    STATE_TYPE_BOOL,    // 布尔状态
    STATE_TYPE_FLOAT,   // 浮点状态
    STATE_TYPE_INT,     // 整数状态
    STATE_TYPE_STRING,  // 字符串状态
    STATE_TYPE_CUSTOM,  // 自定义状态
} StateType;
```

## 内存管理

### 所有权模型
- **Widget**: 由创建者拥有，使用后释放
- **Element**: 由WidgetTree管理，自动释放
- **RenderObject**: 由Element管理，自动释放
- **ReactiveState**: 显式创建和释放

### 内存安全
- 使用智能指针（在支持的语言中）
- 自动引用计数
- RAII模式
- 内存池技术减少碎片

## 性能优化

### 1. Diff算法优化
- **Key匹配**: 使用Key快速识别Widget，避免不必要的重建
- **浅比较**: 首先比较类型和Key，只有需要时才比较属性
- **增量更新**: 只更新变化的部分

### 2. 渲染优化
- **脏标记**: 只标记需要更新的Widget
- **布局缓存**: 缓存布局计算结果
- **虚拟滚动**: 优化长列表渲染

### 3. 状态优化
- **依赖追踪**: 自动追踪状态依赖
- **惰性更新**: 批量处理状态更新
- **防抖**: 避免频繁的重复更新

## FFI接口

### C API
所有公共函数都通过C ABI暴露，确保跨语言兼容性：

```cpp
extern "C" Widget* decl_create_text_widget(const char* text);
extern "C" void decl_set_state_bool(ReactiveState* state, bool value);
```

### 类型映射
| C/C++类型 | Nature-Lang类型 |
|-----------|-----------------|
| bool      | bool            |
| float     | f32             |
| int32_t   | i32             |
| char*     | libc.cstr       |
| void*     | anyptr          |

## 测试策略

### 单元测试
- 测试每个核心功能
- 测试边界条件
- 测试内存泄漏

### 集成测试
- 测试Widget组合
- 测试状态管理
- 测试Diff算法

### 性能测试
- 测试渲染性能
- 测试内存使用
- 测试帧率稳定性

### 测试覆盖
- 响应式状态系统
- Widget创建和组合
- Element生命周期
- RenderObject渲染
- Diff算法正确性
- 内存管理

## 使用示例

### 基本用法
```cpp
// 创建状态
ReactiveState* enabled = create_state_bool(true);

// 创建Widget
Widget* checkbox = create_checkbox_widget("Enable", enabled);

// 创建WidgetTree
WidgetTree* tree = create_widget_tree(checkbox);

// 构建和渲染
build_widget_tree(tree);
render_widget_tree(tree);
```

### 复杂组件
```cpp
Widget* create_panel(void) {
    ReactiveState* volume = create_state_float(0.5f);
    ReactiveState* enabled = create_state_bool(true);
    
    Widget* children[] = {
        create_text_widget("Settings"),
        create_checkbox_widget("Enable", enabled),
        create_slider_widget("Volume", volume, 0.0f, 1.0f),
        create_button_widget("Save", on_save_click),
    };
    
    return create_container_widget(children, 4);
}
```

## 未来扩展

### 计划功能
1. **动画系统**: 支持平滑过渡和动画效果
2. **主题系统**: 支持自定义主题和样式
3. **国际化**: 支持多语言和本地化
4. **无障碍**: 支持屏幕阅读器和辅助技术
5. **组件库**: 提供丰富的预构建组件

### 性能改进
1. **WebWorker**: 在后台线程执行Diff计算
2. **GPU加速**: 使用Compute Shader加速布局计算
3. **对象池**: 复用Widget和Element对象

## 兼容性

### 平台支持
- macOS (ARM64, x86_64)
- Linux (x86_64)
- Windows (x86_64)

### 编译器支持
- GCC >= 9.0
- Clang >= 10.0
- MSVC >= 2019

### 依赖项
- Dear ImGui >= 1.89
- SDL3 >= 3.4.0
- WGPU-Native >= 0.20

## 总结

本声明式UI框架通过分层架构、响应式状态系统和智能Diff算法，在保持Dear ImGui高性能特性的同时，提供了现代化的声明式开发体验。框架设计注重类型安全、内存安全和性能优化，为开发者提供了强大而易用的UI开发工具。
