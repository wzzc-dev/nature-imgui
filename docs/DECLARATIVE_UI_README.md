# 声明式UI框架使用指南

## 简介

本框架为Nature ImGui项目添加了声明式UI开发模式，提供类似SwiftUI和Flutter的开发体验。通过声明式语法、响应式数据绑定和智能Diff算法，您可以更高效地构建复杂的用户界面。

## 快速开始

### 1. 初始化

```cpp
#include "declarative_ui_bindings.h"

// 初始化渲染器
init_declarative_renderer();
```

### 2. 创建第一个声明式UI

```cpp
// 创建状态
ReactiveState* checked = create_state_bool(false);

// 创建Widget
Widget* checkbox = create_checkbox_widget("Enable Feature", checked);

// 创建WidgetTree
WidgetTree* tree = create_widget_tree(checkbox);

// 构建和渲染
build_widget_tree(tree);
render_widget_tree(tree);
```

### 3. 响应式更新

```cpp
// 状态变化会自动触发UI更新
set_state_bool(checked, true);
```

## 核心概念

### Widget（组件）

Widget是不可变的UI描述对象，用于声明UI结构。

#### 创建Widget

```cpp
// 文本
Widget* text = create_text_widget("Hello World");

// 按钮
Widget* button = create_button_widget("Click Me", on_click_callback);

// 复选框
ReactiveState* state = create_state_bool(true);
Widget* checkbox = create_checkbox_widget("Check Me", state);

// 滑块
ReactiveState* slider_state = create_state_float(0.5f);
Widget* slider = create_slider_widget("Volume", slider_state, 0.0f, 1.0f);

// 容器
Widget* children[] = {text, button};
Widget* container = create_container_widget(children, 2);

// 窗口
Widget* window = create_window_widget("My Window", container);
```

### ReactiveState（响应式状态）

状态管理是声明式UI的核心，状态变化会自动触发UI更新。

#### 创建状态

```cpp
// 布尔状态
ReactiveState* bool_state = create_state_bool(false);

// 浮点状态
ReactiveState* float_state = create_state_float(0.5f);

// 整数状态
ReactiveState* int_state = create_state_int(42);

// 字符串状态
ReactiveState* string_state = create_state_string("Hello");
```

#### 读取状态

```cpp
bool value = get_state_bool(bool_state);
float fvalue = get_state_float(float_state);
int ivalue = get_state_int(int_state);
const char* svalue = get_state_string(string_state);
```

#### 更新状态

```cpp
set_state_bool(bool_state, true);
set_state_float(float_state, 0.75f);
set_state_int(int_state, 100);
set_state_string(string_state, "World");
```

#### 状态监听

```cpp
void on_state_changed(ReactiveState* state, void* user_data) {
    printf("State changed!\n");
}

add_state_listener(bool_state, on_state_changed, nullptr);
```

### WidgetTree（组件树）

WidgetTree管理整个UI组件树的生命周期。

#### 创建和构建

```cpp
// 创建
WidgetTree* tree = create_widget_tree(root_widget);

// 构建
build_widget_tree(tree);

// 渲染
render_widget_tree(tree);
```

#### 更新

```cpp
// 更新根Widget
Widget* new_root = create_text_widget("New Content");
update_widget_tree(tree, new_root);
```

#### 释放

```cpp
release_widget_tree(tree);
```

## 完整示例

### 示例1: 简单计数器

```cpp
#include "declarative_ui_bindings.h"

static int g_count = 0;

void on_increment(void* user_data) {
    g_count++;
}

Widget* create_counter() {
    Widget* text = create_text_widget("Counter: 0");
    Widget* button = create_button_widget("Increment", on_increment);
    
    Widget* children[] = {text, button};
    return create_container_widget(children, 2);
}
```

### 示例2: 设置面板

```cpp
Widget* create_settings_panel() {
    // 创建状态
    ReactiveState* enable_feature = create_state_bool(true);
    ReactiveState* volume = create_state_float(0.5f);
    ReactiveState* brightness = create_state_float(0.7f);
    
    // 创建Widgets
    Widget* widgets[] = {
        create_text_widget("Settings"),
        create_checkbox_widget("Enable Feature", enable_feature),
        create_slider_widget("Volume", volume, 0.0f, 1.0f),
        create_slider_widget("Brightness", brightness, 0.0f, 1.0f),
        create_button_widget("Save", on_save),
        create_button_widget("Reset", on_reset),
    };
    
    return create_container_widget(widgets, 6);
}
```

### 示例3: 动态列表

```cpp
Widget* create_list_item(const char* text, int index) {
    char buffer[256];
    sprintf(buffer, "Item %d: %s", index, text);
    return create_text_widget(buffer);
}

Widget* create_dynamic_list(const char** items, int count) {
    Widget** widgets = (Widget**)malloc(count * sizeof(Widget*));
    
    for (int i = 0; i < count; i++) {
        widgets[i] = create_list_item(items[i], i);
    }
    
    Widget* list = create_container_widget(widgets, count);
    free(widgets);
    
    return list;
}
```

## 高级特性

### 1. 嵌套组件

```cpp
Widget* create_nested_ui() {
    Widget* inner_panel = create_container_widget((Widget*[]){
        create_text_widget("Inner Content"),
        create_button_widget("Inner Button", nullptr),
    }, 2);
    
    return create_window_widget("Outer Window",
        create_container_widget((Widget*[]){
            create_text_widget("Outer Content"),
            inner_panel,
            create_button_widget("Outer Button", nullptr),
        }, 3)
    );
}
```

### 2. 条件渲染

```cpp
Widget* create_conditional_ui(bool show_advanced) {
    Widget* basic_widgets[] = {
        create_text_widget("Basic Options"),
        create_button_widget("Ok", nullptr),
    };
    
    if (show_advanced) {
        Widget* advanced_widget = create_text_widget("Advanced Options");
        return create_container_widget(basic_widgets, 2);
    } else {
        return create_container_widget(basic_widgets, 2);
    }
}
```

### 3. 多窗口管理

```cpp
void render_multiple_windows() {
    // 主窗口
    WidgetTree* main_tree = create_widget_tree(create_main_window());
    build_widget_tree(main_tree);
    
    // 设置窗口
    WidgetTree* settings_tree = create_widget_tree(create_settings_window());
    build_widget_tree(settings_tree);
    
    // 仪表板窗口
    WidgetTree* dashboard_tree = create_widget_tree(create_dashboard());
    build_widget_tree(dashboard_tree);
    
    // 在渲染循环中
    while (!done) {
        render_widget_tree(main_tree);
        if (show_settings) {
            render_widget_tree(settings_tree);
        }
        if (show_dashboard) {
            render_widget_tree(dashboard_tree);
        }
    }
}
```

## 性能优化

### 1. 避免不必要的重建

```cpp
// 不好的做法：每次都创建新Widget
while (running) {
    Widget* widget = create_text_widget("Hello");
    render_widget(widget);
    release_widget(widget);
}

// 好的做法：复用Widget
Widget* widget = create_text_widget("Hello");
WidgetTree* tree = create_widget_tree(widget);
build_widget_tree(tree);

while (running) {
    render_widget_tree(tree);
}
```

### 2. 使用状态监听

```cpp
// 添加状态监听，只在状态变化时更新
add_state_listener(state, on_state_changed, nullptr);
```

### 3. 虚拟滚动

```cpp
// 启用虚拟滚动以优化长列表
enable_virtual_scrolling(true);
set_virtual_scroll_viewport(1280.0f, 720.0f);
```

## 调试

### 启用调试模式

```cpp
// 启用渲染调试
enable_render_debug(true);
```

### 打印统计信息

```cpp
// 在每帧结束时调用
print_render_stats();
```

### 打印Widget树

```cpp
// 打印Widget树结构
print_widget_tree(root_widget, 0);
```

## 内存管理

### 自动内存管理

WidgetTree会自动管理其包含的Widget和Element的生命周期：

```cpp
WidgetTree* tree = create_widget_tree(root_widget);
// ... 使用树 ...
release_widget_tree(tree);  // 自动释放所有相关资源
```

### 显式释放

对于独立的Widget和State，需要显式释放：

```cpp
Widget* widget = create_text_widget("Hello");
// ... 使用widget ...
release_widget(widget);

ReactiveState* state = create_state_bool(false);
// ... 使用state ...
release_state(state);
```

## 常见问题

### Q: 如何处理复杂的用户交互？

A: 使用回调函数和状态管理：

```cpp
void on_complex_interaction(void* user_data) {
    // 更新多个状态
    set_state_bool(state1, true);
    set_state_float(state2, 0.5f);
    // ...
}
```

### Q: 如何实现动画效果？

A: 在定时器中更新状态：

```cpp
void animate(void* user_data) {
    float value = get_state_float(state);
    set_state_float(state, value + 0.01f);
}
```

### Q: 如何调试状态变化？

A: 添加状态监听器打印日志：

```cpp
void debug_state_change(ReactiveState* state, void* user_data) {
    printf("State %u changed\n", state->state_id);
}
add_state_listener(state, debug_state_change, nullptr);
```

## 最佳实践

1. **保持Widget纯净**: Widget应该是纯函数式，不包含可变状态
2. **使用状态管理**: 所有可变状态都应该通过ReactiveState管理
3. **组件化设计**: 将复杂UI拆分为小组件，便于复用和维护
4. **性能优化**: 避免在渲染循环中创建新Widget，复用WidgetTree
5. **内存安全**: 及时释放不再使用的Widget和State
6. **类型安全**: 使用正确的类型，避免类型转换
7. **测试覆盖**: 为关键组件编写单元测试

## 参考资料

- [架构设计文档](DECLARATIVE_UI_DESIGN.md)
- [API参考](declarative_ui.h)
- [示例代码](examples/)
- [单元测试](tests/)

## 贡献

欢迎贡献！请遵循项目的贡献指南。

## 许可证

MIT License
