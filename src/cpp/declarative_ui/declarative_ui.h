// declarative_ui.h
// 声明式UI框架核心定义
#ifndef DECLARATIVE_UI_H
#define DECLARATIVE_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 类型定义
// ============================================================================

// 组件类型枚举
typedef enum {
    WIDGET_TYPE_TEXT,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_CHECKBOX,
    WIDGET_TYPE_SLIDER,
    WIDGET_TYPE_CONTAINER,
    WIDGET_TYPE_WINDOW,
    WIDGET_TYPE_UNKNOWN = 0xFF
} WidgetType;

// 属性类型枚举
typedef enum {
    PROP_TYPE_STRING,
    PROP_TYPE_FLOAT,
    PROP_TYPE_BOOL,
    PROP_TYPE_INT,
    PROP_TYPE_CALLBACK,
    PROP_TYPE_WIDGET,
    PROP_TYPE_WIDGET_ARRAY,
    PROP_TYPE_COLOR,
    PROP_TYPE_UNKNOWN = 0xFF
} PropertyType;

// 响应式状态类型
typedef enum {
    STATE_TYPE_BOOL,
    STATE_TYPE_FLOAT,
    STATE_TYPE_INT,
    STATE_TYPE_STRING,
    STATE_TYPE_CUSTOM
} StateType;

// ============================================================================
// 前向声明
// ============================================================================

typedef struct Widget Widget;
typedef struct Property Property;
typedef struct StatefulElement StatefulElement;
typedef struct RenderObject RenderObject;
typedef struct BuildContext BuildContext;
typedef struct ReactiveState ReactiveState;
typedef struct WidgetTree WidgetTree;

// ============================================================================
// 回调函数类型定义
// ============================================================================

// 属性释放回调
typedef void (*PropertyReleaseFn)(Property* prop);

// Widget构建回调
typedef Widget* (*WidgetBuilderFn)(BuildContext* context, void* user_data);

// 状态更新回调
typedef void (*StateUpdateFn)(ReactiveState* state, void* new_value);

// 状态变更监听器
typedef void (*StateChangeListener)(ReactiveState* state, void* user_data);

// 事件处理回调
typedef void (*EventHandlerFn)(void* user_data);

// ============================================================================
// 响应式状态系统
// ============================================================================

// 响应式状态结构
struct ReactiveState {
    StateType type;
    union {
        bool bool_value;
        float float_value;
        int32_t int_value;
        char* string_value;
        void* custom_value;
    } current;
    
    // 依赖追踪
    StateChangeListener* listeners;
    size_t listener_count;
    size_t listener_capacity;
    
    // 状态ID
    uint32_t state_id;
    
    // 内存管理
    void* memory_pool;
    size_t pool_size;
    size_t pool_offset;
};

// 创建响应式状态
ReactiveState* create_state_bool(bool initial_value);
ReactiveState* create_state_float(float initial_value);
ReactiveState* create_state_int(int32_t initial_value);
ReactiveState* create_state_string(const char* initial_value);

// 获取状态值
bool get_state_bool(ReactiveState* state);
float get_state_float(ReactiveState* state);
int32_t get_state_int(ReactiveState* state);
const char* get_state_string(ReactiveState* state);

// 设置状态值（触发更新）
void set_state_bool(ReactiveState* state, bool value);
void set_state_float(ReactiveState* state, float value);
void set_state_int(ReactiveState* state, int32_t value);
void set_state_string(ReactiveState* state, const char* value);

// 添加监听器
void add_state_listener(ReactiveState* state, StateChangeListener listener, void* user_data);

// 移除监听器
void remove_state_listener(ReactiveState* state, StateChangeListener listener);

// 释放状态
void release_state(ReactiveState* state);

// ============================================================================
// 属性系统
// ============================================================================

// 属性结构
struct Property {
    const char* name;
    PropertyType type;
    union {
        char* string_value;
        float float_value;
        bool bool_value;
        int32_t int_value;
        EventHandlerFn callback;
        Widget* widget_value;
        struct {
            Widget** widgets;
            size_t count;
        } widget_array;
        struct {
            float r, g, b, a;
        } color_value;
    } value;
    
    PropertyReleaseFn release_fn;
    
    // 引用计数（用于多个对象共享同一个属性）
    int ref_count;
};

// 创建属性
Property* create_string_property(const char* name, const char* value);
Property* create_float_property(const char* name, float value);
Property* create_bool_property(const char* name, bool value);
Property* create_int_property(const char* name, int32_t value);
Property* create_callback_property(const char* name, EventHandlerFn callback);
Property* create_widget_property(const char* name, Widget* widget);
Property* create_widget_array_property(const char* name, Widget** widgets, size_t count);
Property* create_color_property(const char* name, float r, float g, float b, float a);

// 设置属性值
void set_property_string(Property* prop, const char* value);
void set_property_float(Property* prop, float value);
void set_property_bool(Property* prop, bool value);
void set_property_int(Property* prop, int32_t value);

// 获取属性值
const char* get_property_string(const Property* prop);
float get_property_float(const Property* prop);
bool get_property_bool(const Property* prop);
int32_t get_property_int(const Property* prop);

// 释放属性
void release_property(Property* prop);

// ============================================================================
// Widget系统
// ============================================================================

// Widget基类结构
struct Widget {
    WidgetType type;
    
    // 属性列表
    Property** properties;
    size_t property_count;
    size_t property_capacity;
    
    // 子组件
    Widget** children;
    size_t child_count;
    size_t child_capacity;
    
    // Key用于Diff算法
    const char* key;
    
    // Widget ID
    uint32_t widget_id;
    
    // 内存管理
    void* (*clone)(const Widget* widget);
    void (*release)(Widget* widget);
};

// 创建Widget
Widget* create_text_widget(const char* text);
Widget* create_button_widget(const char* label, EventHandlerFn on_click);
Widget* create_checkbox_widget(const char* label, ReactiveState* state);
Widget* create_slider_widget(const char* label, ReactiveState* state, float min, float max);
Widget* create_container_widget(Widget** children, size_t count);
Widget* create_window_widget(const char* title, Widget* child);

// Widget属性操作
void widget_add_property(Widget* widget, Property* prop);
Property* widget_get_property(const Widget* widget, const char* name);
void widget_add_child(Widget* widget, Widget* child);

// Widget克隆和释放
Widget* clone_widget(const Widget* widget);
void release_widget(Widget* widget);

// Widget比较（用于Diff算法）
bool widgets_equal(const Widget* a, const Widget* b);
bool widgets_can_update(const Widget* old_widget, const Widget* new_widget);

// ============================================================================
// Element系统（运行时）
// ============================================================================

// Element类型
typedef enum {
    ELEMENT_TYPE_STATELESS,
    ELEMENT_TYPE_STATEFUL,
    ELEMENT_TYPE_RENDER
} ElementType;

// Element基类
struct StatefulElement {
    ElementType type;
    Widget* widget;
    
    // 对应的RenderObject
    RenderObject* render_object;
    
    // 父Element
    struct StatefulElement* parent;
    
    // 子Elements
    StatefulElement** children;
    size_t child_count;
    size_t child_capacity;
    
    // 构建上下文
    BuildContext* build_context;
    
    // Element ID
    uint32_t element_id;
    
    // 生命周期标记
    bool is_mounted;
    bool is_dirty;
};

// 创建Element
StatefulElement* create_element(Widget* widget, StatefulElement* parent);
void mount_element(StatefulElement* element, BuildContext* context);
void update_element(StatefulElement* element, Widget* new_widget);
void unmount_element(StatefulElement* element);
void release_element(StatefulElement* element);

// ============================================================================
// RenderObject系统（渲染）
// ============================================================================

struct RenderObject {
    WidgetType widget_type;
    
    // 属性缓存
    Property** properties;
    size_t property_count;
    
    // 渲染状态
    bool needs_layout;
    bool needs_paint;
    
    // 子RenderObjects
    RenderObject** children;
    size_t child_count;
    size_t child_capacity;
    
    // 关联的Element
    StatefulElement* element;
    
    // 渲染方法
    void (*perform_layout)(RenderObject* obj);
    void (*paint)(RenderObject* obj);
    
    uint32_t render_object_id;
};

// 创建RenderObject
RenderObject* create_render_object(WidgetType type);
void attach_render_object(RenderObject* render_obj, StatefulElement* element);
void detach_render_object(RenderObject* render_obj);
void release_render_object(RenderObject* render_obj);

// ============================================================================
// BuildContext（构建上下文）
// ============================================================================

struct BuildContext {
    WidgetTree* widget_tree;
    
    // 当前Element
    StatefulElement* current_element;
    
    // 状态存储
    ReactiveState** states;
    size_t state_count;
    
    // 全局状态
    void* global_state;
    
    // 依赖注入容器
    void* dependency_injection;
};

// 创建BuildContext
BuildContext* create_build_context(WidgetTree* tree);
void build_context_inherit(BuildContext* child, BuildContext* parent);
void release_build_context(BuildContext* context);

// ============================================================================
// WidgetTree（组件树）
// ============================================================================

struct WidgetTree {
    Widget* root_widget;
    StatefulElement* root_element;
    BuildContext* build_context;
    
    // Dirty标记
    bool needs_rebuild;
    
    // 布局和绘制标志
    bool needs_layout;
    bool needs_paint;
    
    // 树ID
    uint32_t tree_id;
};

// 创建WidgetTree
WidgetTree* create_widget_tree(Widget* root_widget);

// 构建和更新WidgetTree
void build_widget_tree(WidgetTree* tree);
void update_widget_tree(WidgetTree* tree, Widget* new_root);

// 布局和绘制
void layout_widget_tree(WidgetTree* tree);
void paint_widget_tree(WidgetTree* tree);

// 释放WidgetTree
void release_widget_tree(WidgetTree* tree);

// ============================================================================
// Diff算法
// ============================================================================

// Diff结果
typedef struct {
    // 需要更新的Widgets
    Widget** updated_widgets;
    size_t update_count;
    
    // 需要移除的Widgets
    Widget** removed_widgets;
    size_t remove_count;
    
    // 需要添加的Widgets
    Widget** added_widgets;
    size_t add_count;
    
    // 需要移动的Widgets
    struct {
        Widget* widget;
        size_t old_index;
        size_t new_index;
    }* moved_widgets;
    size_t move_count;
} DiffResult;

// 执行Diff算法
DiffResult* diff_widgets(const Widget* old_widget, const Widget* new_widget);
void apply_diff(WidgetTree* tree, DiffResult* diff);
void release_diff_result(DiffResult* diff);

// ============================================================================
// 便利宏
// ============================================================================

// 创建Text Widget
#define TEXT(text) create_text_widget(text)

// 创建Button Widget
#define BUTTON(label, callback) create_button_widget(label, callback)

// 创建Checkbox Widget
#define CHECKBOX(label, state) create_checkbox_widget(label, state)

// 创建Slider Widget
#define SLIDER(label, state, min, max) create_slider_widget(label, state, min, max)

// 创建Container Widget
#define CONTAINER(...) ({ \
    Widget* children[] = {__VA_ARGS__}; \
    create_container_widget(children, sizeof(children) / sizeof(children[0])); \
})

// 创建Window Widget
#define WINDOW(title, child) create_window_widget(title, child)

#ifdef __cplusplus
}
#endif

#endif // DECLARATIVE_UI_H
