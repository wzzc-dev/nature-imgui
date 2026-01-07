// declarative_ui.cpp
// 声明式UI框架核心实现

#include "declarative_ui.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// 全局ID计数器
// ============================================================================

static uint32_t g_next_widget_id = 1;
static uint32_t g_next_element_id = 1;
static uint32_t g_next_render_object_id = 1;
static uint32_t g_next_state_id = 1;

// ============================================================================
// 内存池管理（用于减少碎片化）
// ============================================================================

#define MEMORY_POOL_SIZE (1024 * 1024) // 1MB

typedef struct {
    void* pool;
    size_t offset;
    size_t capacity;
} MemoryPool;

static MemoryPool* g_main_pool = nullptr;

void* pool_alloc(MemoryPool* pool, size_t size) {
    size_t aligned_size = (size + 7) & ~7; // 8字节对齐
    if (pool->offset + aligned_size > pool->capacity) {
        // 池满了，使用malloc
        return malloc(size);
    }
    void* ptr = (char*)pool->pool + pool->offset;
    pool->offset += aligned_size;
    return ptr;
}

void pool_free(MemoryPool* pool, void* ptr) {
    // 简化实现：不实际释放，在池销毁时统一释放
}

// ============================================================================
// 响应式状态系统实现
// ============================================================================

ReactiveState* create_state_bool(bool initial_value) {
    ReactiveState* state = (ReactiveState*)malloc(sizeof(ReactiveState));
    state->type = STATE_TYPE_BOOL;
    state->current.bool_value = initial_value;
    state->listeners = nullptr;
    state->listener_count = 0;
    state->listener_capacity = 0;
    state->state_id = g_next_state_id++;
    state->memory_pool = nullptr;
    state->pool_size = 0;
    state->pool_offset = 0;
    return state;
}

ReactiveState* create_state_float(float initial_value) {
    ReactiveState* state = (ReactiveState*)malloc(sizeof(ReactiveState));
    state->type = STATE_TYPE_FLOAT;
    state->current.float_value = initial_value;
    state->listeners = nullptr;
    state->listener_count = 0;
    state->listener_capacity = 0;
    state->state_id = g_next_state_id++;
    state->memory_pool = nullptr;
    state->pool_size = 0;
    state->pool_offset = 0;
    return state;
}

ReactiveState* create_state_int(int32_t initial_value) {
    ReactiveState* state = (ReactiveState*)malloc(sizeof(ReactiveState));
    state->type = STATE_TYPE_INT;
    state->current.int_value = initial_value;
    state->listeners = nullptr;
    state->listener_count = 0;
    state->listener_capacity = 0;
    state->state_id = g_next_state_id++;
    state->memory_pool = nullptr;
    state->pool_size = 0;
    state->pool_offset = 0;
    return state;
}

ReactiveState* create_state_string(const char* initial_value) {
    ReactiveState* state = (ReactiveState*)malloc(sizeof(ReactiveState));
    state->type = STATE_TYPE_STRING;
    if (initial_value) {
        state->current.string_value = strdup(initial_value);
    } else {
        state->current.string_value = nullptr;
    }
    state->listeners = nullptr;
    state->listener_count = 0;
    state->listener_capacity = 0;
    state->state_id = g_next_state_id++;
    state->memory_pool = nullptr;
    state->pool_size = 0;
    state->pool_offset = 0;
    return state;
}

bool get_state_bool(ReactiveState* state) {
    if (state->type != STATE_TYPE_BOOL) {
        printf("Error: State type mismatch\n");
        return false;
    }
    return state->current.bool_value;
}

float get_state_float(ReactiveState* state) {
    if (state->type != STATE_TYPE_FLOAT) {
        printf("Error: State type mismatch\n");
        return 0.0f;
    }
    return state->current.float_value;
}

int32_t get_state_int(ReactiveState* state) {
    if (state->type != STATE_TYPE_INT) {
        printf("Error: State type mismatch\n");
        return 0;
    }
    return state->current.int_value;
}

const char* get_state_string(ReactiveState* state) {
    if (state->type != STATE_TYPE_STRING) {
        printf("Error: State type mismatch\n");
        return nullptr;
    }
    return state->current.string_value;
}

void set_state_bool(ReactiveState* state, bool value) {
    if (state->type != STATE_TYPE_BOOL) {
        printf("Error: State type mismatch\n");
        return;
    }
    if (state->current.bool_value == value) {
        return; // 值未改变，不触发更新
    }
    state->current.bool_value = value;
    
    // 通知所有监听器
    for (size_t i = 0; i < state->listener_count; i++) {
        if (state->listeners[i]) {
            state->listeners[i](state, nullptr);
        }
    }
}

void set_state_float(ReactiveState* state, float value) {
    if (state->type != STATE_TYPE_FLOAT) {
        printf("Error: State type mismatch\n");
        return;
    }
    if (state->current.float_value == value) {
        return;
    }
    state->current.float_value = value;
    
    for (size_t i = 0; i < state->listener_count; i++) {
        if (state->listeners[i]) {
            state->listeners[i](state, nullptr);
        }
    }
}

void set_state_int(ReactiveState* state, int32_t value) {
    if (state->type != STATE_TYPE_INT) {
        printf("Error: State type mismatch\n");
        return;
    }
    if (state->current.int_value == value) {
        return;
    }
    state->current.int_value = value;
    
    for (size_t i = 0; i < state->listener_count; i++) {
        if (state->listeners[i]) {
            state->listeners[i](state, nullptr);
        }
    }
}

void set_state_string(ReactiveState* state, const char* value) {
    if (state->type != STATE_TYPE_STRING) {
        printf("Error: State type mismatch\n");
        return;
    }
    if (state->current.string_value && value && 
        strcmp(state->current.string_value, value) == 0) {
        return;
    }
    
    if (state->current.string_value) {
        free(state->current.string_value);
    }
    state->current.string_value = value ? strdup(value) : nullptr;
    
    for (size_t i = 0; i < state->listener_count; i++) {
        if (state->listeners[i]) {
            state->listeners[i](state, nullptr);
        }
    }
}

void add_state_listener(ReactiveState* state, StateChangeListener listener, void* user_data) {
    // 暂时忽略user_data，简化实现
    if (state->listener_count >= state->listener_capacity) {
        size_t new_capacity = state->listener_capacity == 0 ? 4 : state->listener_capacity * 2;
        StateChangeListener* new_listeners = (StateChangeListener*)realloc(
            state->listeners, new_capacity * sizeof(StateChangeListener)
        );
        if (!new_listeners) {
            printf("Error: Failed to allocate memory for listeners\n");
            return;
        }
        state->listeners = new_listeners;
        state->listener_capacity = new_capacity;
    }
    state->listeners[state->listener_count++] = listener;
}

void remove_state_listener(ReactiveState* state, StateChangeListener listener) {
    for (size_t i = 0; i < state->listener_count; i++) {
        if (state->listeners[i] == listener) {
            // 移除监听器
            for (size_t j = i; j < state->listener_count - 1; j++) {
                state->listeners[j] = state->listeners[j + 1];
            }
            state->listener_count--;
            return;
        }
    }
}

void release_state(ReactiveState* state) {
    if (!state) return;
    
    if (state->type == STATE_TYPE_STRING && state->current.string_value) {
        free(state->current.string_value);
    }
    
    if (state->listeners) {
        free(state->listeners);
    }
    
    free(state);
}

// ============================================================================
// 属性系统实现
// ============================================================================

Property* create_string_property(const char* name, const char* value) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_STRING;
    prop->value.string_value = value ? strdup(value) : nullptr;
    prop->release_fn = nullptr;
    prop->ref_count = 1; // 初始化引用计数为1
    return prop;
}

Property* create_float_property(const char* name, float value) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_FLOAT;
    prop->value.float_value = value;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_bool_property(const char* name, bool value) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_BOOL;
    prop->value.bool_value = value;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_int_property(const char* name, int32_t value) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_INT;
    prop->value.int_value = value;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_callback_property(const char* name, EventHandlerFn callback) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_CALLBACK;
    prop->value.callback = callback;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_widget_property(const char* name, Widget* widget) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_WIDGET;
    prop->value.widget_value = widget;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_widget_array_property(const char* name, Widget** widgets, size_t count) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_WIDGET_ARRAY;
    prop->value.widget_array.widgets = widgets;
    prop->value.widget_array.count = count;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

Property* create_color_property(const char* name, float r, float g, float b, float a) {
    Property* prop = (Property*)malloc(sizeof(Property));
    prop->name = name ? strdup(name) : nullptr;
    prop->type = PROP_TYPE_COLOR;
    prop->value.color_value.r = r;
    prop->value.color_value.g = g;
    prop->value.color_value.b = b;
    prop->value.color_value.a = a;
    prop->release_fn = nullptr;
    prop->ref_count = 1;
    return prop;
}

void set_property_string(Property* prop, const char* value) {
    if (prop->type != PROP_TYPE_STRING) {
        printf("Error: Property type mismatch\n");
        return;
    }
    if (prop->value.string_value) {
        free(prop->value.string_value);
    }
    prop->value.string_value = value ? strdup(value) : nullptr;
}

void set_property_float(Property* prop, float value) {
    if (prop->type != PROP_TYPE_FLOAT) {
        printf("Error: Property type mismatch\n");
        return;
    }
    prop->value.float_value = value;
}

void set_property_bool(Property* prop, bool value) {
    if (prop->type != PROP_TYPE_BOOL) {
        printf("Error: Property type mismatch\n");
        return;
    }
    prop->value.bool_value = value;
}

void set_property_int(Property* prop, int32_t value) {
    if (prop->type != PROP_TYPE_INT) {
        printf("Error: Property type mismatch\n");
        return;
    }
    prop->value.int_value = value;
}

const char* get_property_string(const Property* prop) {
    if (prop->type != PROP_TYPE_STRING) {
        printf("Error: Property type mismatch\n");
        return nullptr;
    }
    return prop->value.string_value;
}

float get_property_float(const Property* prop) {
    if (prop->type != PROP_TYPE_FLOAT) {
        printf("Error: Property type mismatch\n");
        return 0.0f;
    }
    return prop->value.float_value;
}

bool get_property_bool(const Property* prop) {
    if (prop->type != PROP_TYPE_BOOL) {
        printf("Error: Property type mismatch\n");
        return false;
    }
    return prop->value.bool_value;
}

int32_t get_property_int(const Property* prop) {
    if (prop->type != PROP_TYPE_INT) {
        printf("Error: Property type mismatch\n");
        return 0;
    }
    return prop->value.int_value;
}

void release_property(Property* prop) {
    if (!prop) return;
    
    // 减少引用计数
    prop->ref_count--;
    
    // 如果引用计数大于0，不要释放
    if (prop->ref_count > 0) {
        return;
    }
    
    // 引用计数为0，真正释放内存
    if (prop->name) {
        free((void*)prop->name);
    }
    
    if (prop->type == PROP_TYPE_STRING && prop->value.string_value) {
        free(prop->value.string_value);
    }
    
    free(prop);
}

// ============================================================================
// Widget系统实现
// ============================================================================

static void* widget_base_clone(const Widget* widget) { return nullptr; }
static void widget_base_release(Widget* widget) {}

Widget* create_widget_base(WidgetType type) {
    Widget* widget = (Widget*)malloc(sizeof(Widget));
    widget->type = type;
    widget->properties = nullptr;
    widget->property_count = 0;
    widget->property_capacity = 0;
    widget->children = nullptr;
    widget->child_count = 0;
    widget->child_capacity = 0;
    widget->key = nullptr;
    widget->widget_id = g_next_widget_id++;
    widget->clone = widget_base_clone;
    widget->release = widget_base_release;
    return widget;
}

Widget* create_text_widget(const char* text) {
    Widget* widget = create_widget_base(WIDGET_TYPE_TEXT);
    Property* text_prop = create_string_property("text", text);
    widget_add_property(widget, text_prop);
    return widget;
}

Widget* create_button_widget(const char* label, EventHandlerFn on_click) {
    Widget* widget = create_widget_base(WIDGET_TYPE_BUTTON);
    Property* label_prop = create_string_property("label", label);
    Property* callback_prop = create_callback_property("on_click", on_click);
    widget_add_property(widget, label_prop);
    widget_add_property(widget, callback_prop);
    return widget;
}

Widget* create_checkbox_widget(const char* label, ReactiveState* state) {
    Widget* widget = create_widget_base(WIDGET_TYPE_CHECKBOX);
    Property* label_prop = create_string_property("label", label);
    Property* state_prop = create_widget_property("state", (Widget*)state); // 存储state指针
    widget_add_property(widget, label_prop);
    widget_add_property(widget, state_prop);
    return widget;
}

Widget* create_slider_widget(const char* label, ReactiveState* state, float min, float max) {
    Widget* widget = create_widget_base(WIDGET_TYPE_SLIDER);
    Property* label_prop = create_string_property("label", label);
    Property* state_prop = create_widget_property("state", (Widget*)state);
    Property* min_prop = create_float_property("min", min);
    Property* max_prop = create_float_property("max", max);
    widget_add_property(widget, label_prop);
    widget_add_property(widget, state_prop);
    widget_add_property(widget, min_prop);
    widget_add_property(widget, max_prop);
    return widget;
}

Widget* create_container_widget(Widget** children, size_t count) {
    Widget* widget = create_widget_base(WIDGET_TYPE_CONTAINER);
    for (size_t i = 0; i < count; i++) {
        widget_add_child(widget, children[i]);
    }
    return widget;
}

Widget* create_window_widget(const char* title, Widget* child) {
    Widget* widget = create_widget_base(WIDGET_TYPE_WINDOW);
    Property* title_prop = create_string_property("title", title);
    widget_add_property(widget, title_prop);
    if (child) {
        widget_add_child(widget, child);
    }
    return widget;
}

void widget_add_property(Widget* widget, Property* prop) {
    if (widget->property_count >= widget->property_capacity) {
        size_t new_capacity = widget->property_capacity == 0 ? 4 : widget->property_capacity * 2;
        Property** new_properties = (Property**)realloc(
            widget->properties, new_capacity * sizeof(Property*)
        );
        if (!new_properties) {
            printf("Error: Failed to allocate memory for properties\n");
            release_property(prop);
            return;
        }
        widget->properties = new_properties;
        widget->property_capacity = new_capacity;
    }
    widget->properties[widget->property_count++] = prop;
}

Property* widget_get_property(const Widget* widget, const char* name) {
    for (size_t i = 0; i < widget->property_count; i++) {
        if (widget->properties[i]->name && 
            strcmp(widget->properties[i]->name, name) == 0) {
            return widget->properties[i];
        }
    }
    return nullptr;
}

void widget_add_child(Widget* widget, Widget* child) {
    if (widget->child_count >= widget->child_capacity) {
        size_t new_capacity = widget->child_capacity == 0 ? 4 : widget->child_capacity * 2;
        Widget** new_children = (Widget**)realloc(
            widget->children, new_capacity * sizeof(Widget*)
        );
        if (!new_children) {
            printf("Error: Failed to allocate memory for children\n");
            return;
        }
        widget->children = new_children;
        widget->child_capacity = new_capacity;
    }
    widget->children[widget->child_count++] = child;
}

void release_widget(Widget* widget) {
    if (!widget) return;

    // 释放属性
    for (size_t i = 0; i < widget->property_count; i++) {
        Property* prop = widget->properties[i];
        // 处理 ReactiveState 的释放（通过 "state" 属性名识别）
        if (prop->name && strcmp(prop->name, "state") == 0 && prop->type == PROP_TYPE_WIDGET) {
            // 这是一个 ReactiveState 指针
            release_state((ReactiveState*)prop->value.widget_value);
            prop->value.widget_value = nullptr;
        }
        release_property(prop);
    }
    if (widget->properties) {
        free(widget->properties);
    }

    // 递归释放子组件
    for (size_t i = 0; i < widget->child_count; i++) {
        release_widget(widget->children[i]);
    }
    if (widget->children) {
        free(widget->children);
    }

    if (widget->key) {
        free((void*)widget->key);
    }

    free(widget);
}

bool widgets_equal(const Widget* a, const Widget* b) {
    if (!a || !b) return a == b;
    if (a->type != b->type) return false;
    if (a->widget_id == b->widget_id) return true;
    return false;
}

bool widgets_can_update(const Widget* old_widget, const Widget* new_widget) {
    if (!old_widget || !new_widget) return false;
    return old_widget->type == new_widget->type && 
           old_widget->key == new_widget->key;
}

// ============================================================================
// Element系统实现
// ============================================================================

StatefulElement* create_element(Widget* widget, StatefulElement* parent) {
    StatefulElement* element = (StatefulElement*)malloc(sizeof(StatefulElement));
    element->type = ELEMENT_TYPE_STATELESS; // 默认为无状态
    element->widget = widget;
    element->render_object = nullptr;
    element->parent = parent;
    element->children = nullptr;
    element->child_count = 0;
    element->child_capacity = 0;
    element->build_context = nullptr;
    element->element_id = g_next_element_id++;
    element->is_mounted = false;
    element->is_dirty = false;
    return element;
}

void mount_element(StatefulElement* element, BuildContext* context) {
    if (!element) return;
    
    element->build_context = context;
    element->is_mounted = true;
    
    // 创建RenderObject
    element->render_object = create_render_object(element->widget->type);
    attach_render_object(element->render_object, element);
    
    // 递归挂载子元素
    for (size_t i = 0; i < element->widget->child_count; i++) {
        StatefulElement* child_element = create_element(
            element->widget->children[i], element
        );
        mount_element(child_element, context);
        
        // 添加到children数组
        if (element->child_count >= element->child_capacity) {
            size_t new_capacity = element->child_capacity == 0 ? 4 : element->child_capacity * 2;
            StatefulElement** new_children = (StatefulElement**)realloc(
                element->children, new_capacity * sizeof(StatefulElement*)
            );
            if (!new_children) {
                printf("Error: Failed to allocate memory for children\n");
                continue;
            }
            element->children = new_children;
            element->child_capacity = new_capacity;
        }
        element->children[element->child_count++] = child_element;
    }
}

void unmount_element(StatefulElement* element) {
    if (!element) return;
    
    // 递归卸载子元素
    for (size_t i = 0; i < element->child_count; i++) {
        unmount_element(element->children[i]);
    }
    
    // 卸载RenderObject
    if (element->render_object) {
        detach_render_object(element->render_object);
        release_render_object(element->render_object);
    }
    
    element->is_mounted = false;
}

void release_element(StatefulElement* element) {
    if (!element) return;
    
    unmount_element(element);
    
    // 释放子元素
    for (size_t i = 0; i < element->child_count; i++) {
        release_element(element->children[i]);
    }
    
    if (element->children) {
        free(element->children);
    }
    
    free(element);
}

// ============================================================================
// RenderObject系统实现
// ============================================================================

static void default_perform_layout(RenderObject* obj) {}
static void default_paint(RenderObject* obj) {}

RenderObject* create_render_object(WidgetType type) {
    RenderObject* render_obj = (RenderObject*)malloc(sizeof(RenderObject));
    render_obj->widget_type = type;
    render_obj->properties = nullptr;
    render_obj->property_count = 0;
    render_obj->needs_layout = true;
    render_obj->needs_paint = true;
    render_obj->children = nullptr;
    render_obj->child_count = 0;
    render_obj->child_capacity = 0;
    render_obj->element = nullptr;
    render_obj->perform_layout = default_perform_layout;
    render_obj->paint = default_paint;
    render_obj->render_object_id = g_next_render_object_id++;
    return render_obj;
}

void attach_render_object(RenderObject* render_obj, StatefulElement* element) {
    if (!render_obj || !element) return;
    render_obj->element = element;

    // 复制属性（增加引用计数）
    for (size_t i = 0; i < element->widget->property_count; i++) {
        Property* prop = element->widget->properties[i];
        // 共享引用，增加引用计数
        prop->ref_count++;
        render_obj->properties = (Property**)realloc(
            render_obj->properties,
            (render_obj->property_count + 1) * sizeof(Property*)
        );
        render_obj->properties[render_obj->property_count++] = prop;
    }
}

void detach_render_object(RenderObject* render_obj) {
    if (!render_obj) return;
    render_obj->element = nullptr;
}

void release_render_object(RenderObject* render_obj) {
    if (!render_obj) return;
    
    // 释放子RenderObjects
    for (size_t i = 0; i < render_obj->child_count; i++) {
        release_render_object(render_obj->children[i]);
    }
    
    if (render_obj->children) {
        free(render_obj->children);
    }
    
    // 不释放properties，因为它们由Widget管理
    
    free(render_obj);
}

// ============================================================================
// BuildContext实现
// ============================================================================

BuildContext* create_build_context(WidgetTree* tree) {
    BuildContext* context = (BuildContext*)malloc(sizeof(BuildContext));
    context->widget_tree = tree;
    context->current_element = nullptr;
    context->states = nullptr;
    context->state_count = 0;
    context->global_state = nullptr;
    context->dependency_injection = nullptr;
    return context;
}

void build_context_inherit(BuildContext* child, BuildContext* parent) {
    child->widget_tree = parent->widget_tree;
    child->global_state = parent->global_state;
    child->dependency_injection = parent->dependency_injection;
}

void release_build_context(BuildContext* context) {
    if (!context) return;
    
    // 释放states
    for (size_t i = 0; i < context->state_count; i++) {
        release_state(context->states[i]);
    }
    if (context->states) {
        free(context->states);
    }
    
    free(context);
}

// ============================================================================
// WidgetTree实现
// ============================================================================

WidgetTree* create_widget_tree(Widget* root_widget) {
    WidgetTree* tree = (WidgetTree*)malloc(sizeof(WidgetTree));
    tree->root_widget = root_widget;
    tree->root_element = nullptr;
    tree->build_context = create_build_context(tree);
    tree->needs_rebuild = true;
    tree->needs_layout = true;
    tree->needs_paint = true;
    tree->tree_id = g_next_widget_id++;
    return tree;
}

void build_widget_tree(WidgetTree* tree) {
    if (!tree || !tree->root_widget) return;
    
    if (tree->root_element) {
        // 更新现有树
        update_element(tree->root_element, tree->root_widget);
    } else {
        // 创建新树
        tree->root_element = create_element(tree->root_widget, nullptr);
        mount_element(tree->root_element, tree->build_context);
    }
    
    tree->needs_rebuild = false;
    tree->needs_layout = true;
    tree->needs_paint = true;
}

void update_element(StatefulElement* element, Widget* new_widget) {
    if (!element || !new_widget) return;
    
    if (widgets_can_update(element->widget, new_widget)) {
        // 可以就地更新
        element->widget = new_widget;
        
        // 更新RenderObject属性
        if (element->render_object) {
            // 清空旧属性
            element->render_object->property_count = 0;
            
            // 添加新属性
            for (size_t i = 0; i < new_widget->property_count; i++) {
                Property* prop = new_widget->properties[i];
                element->render_object->properties = (Property**)realloc(
                    element->render_object->properties,
                    (element->render_object->property_count + 1) * sizeof(Property*)
                );
                element->render_object->properties[element->render_object->property_count++] = prop;
            }
        }
        
        element->is_dirty = true;
        
        // 递归更新子元素
        size_t child_index = 0;
        for (size_t i = 0; i < new_widget->child_count; i++) {
            if (child_index < element->child_count) {
                update_element(element->children[child_index], new_widget->children[i]);
            } else {
                // 新增子元素
                StatefulElement* new_child = create_element(new_widget->children[i], element);
                mount_element(new_child, element->build_context);
                
                if (element->child_count >= element->child_capacity) {
                    size_t new_capacity = element->child_capacity == 0 ? 4 : element->child_capacity * 2;
                    StatefulElement** new_children = (StatefulElement**)realloc(
                        element->children, new_capacity * sizeof(StatefulElement*)
                    );
                    if (!new_children) {
                        printf("Error: Failed to allocate memory for children\n");
                        continue;
                    }
                    element->children = new_children;
                    element->child_capacity = new_capacity;
                }
                element->children[element->child_count++] = new_child;
            }
            child_index++;
        }
        
        // 移除多余的子元素
        while (child_index < element->child_count) {
            unmount_element(element->children[child_index]);
            release_element(element->children[child_index]);
            element->child_count--;
        }
    } else {
        // 需要完全重建
        unmount_element(element);
        
        // 更新widget
        element->widget = new_widget;
        
        // 重新挂载
        mount_element(element, element->build_context);
    }
}

void update_widget_tree(WidgetTree* tree, Widget* new_root) {
    if (!tree || !new_root) return;
    
    tree->root_widget = new_root;
    tree->needs_rebuild = true;
    
    build_widget_tree(tree);
}

void layout_widget_tree(WidgetTree* tree) {
    if (!tree || !tree->needs_layout) return;
    
    // 执行布局
    // 这里需要实现具体的布局算法
    // 暂时使用空实现
    
    tree->needs_layout = false;
}

void paint_widget_tree(WidgetTree* tree) {
    if (!tree || !tree->needs_paint) return;
    
    // 执行绘制
    // 这里需要实现具体的绘制逻辑
    // 暂时使用空实现
    
    tree->needs_paint = false;
}

void release_widget_tree(WidgetTree* tree) {
    if (!tree) return;
    
    if (tree->root_element) {
        release_element(tree->root_element);
    }
    
    if (tree->root_widget) {
        release_widget(tree->root_widget);
    }
    
    if (tree->build_context) {
        release_build_context(tree->build_context);
    }
    
    free(tree);
}

// ============================================================================
// Diff算法实现
// ============================================================================

DiffResult* diff_widgets(const Widget* old_widget, const Widget* new_widget) {
    DiffResult* diff = (DiffResult*)malloc(sizeof(DiffResult));
    diff->updated_widgets = nullptr;
    diff->update_count = 0;
    diff->removed_widgets = nullptr;
    diff->remove_count = 0;
    diff->added_widgets = nullptr;
    diff->add_count = 0;
    diff->moved_widgets = nullptr;
    diff->move_count = 0;
    
    // 简化实现：检查是否可以就地更新
    if (widgets_can_update(old_widget, new_widget)) {
        // 可以更新
        diff->updated_widgets = (Widget**)malloc(sizeof(Widget*));
        diff->updated_widgets[0] = (Widget*)new_widget;
        diff->update_count = 1;
    } else {
        // 需要移除和添加
        diff->removed_widgets = (Widget**)malloc(sizeof(Widget*));
        diff->removed_widgets[0] = (Widget*)old_widget;
        diff->remove_count = 1;
        
        diff->added_widgets = (Widget**)malloc(sizeof(Widget*));
        diff->added_widgets[0] = (Widget*)new_widget;
        diff->add_count = 1;
    }
    
    return diff;
}

void apply_diff(WidgetTree* tree, DiffResult* diff) {
    if (!tree || !diff) return;
    
    tree->needs_rebuild = true;
    build_widget_tree(tree);
}

void release_diff_result(DiffResult* diff) {
    if (!diff) return;
    
    if (diff->updated_widgets) {
        free(diff->updated_widgets);
    }
    if (diff->removed_widgets) {
        free(diff->removed_widgets);
    }
    if (diff->added_widgets) {
        free(diff->added_widgets);
    }
    if (diff->moved_widgets) {
        free(diff->moved_widgets);
    }
    
    free(diff);
}
