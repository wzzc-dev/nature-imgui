// declarative_renderer.cpp
// 声明式UI到Dear ImGui的渲染桥接实现

#include "declarative_renderer.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// 全局渲染器状态
// ============================================================================

static RendererState g_renderer_state = {
    .current_tree = nullptr,
    .rendered_widgets = 0,
    .skipped_widgets = 0,
    .cursor_pos = {0, 0},
    .window_size = {0, 0},
    .item_width = 0,
    .dpi_scale = 1.0f,
    .last_frame_time = 0,
    .avg_frame_time = 0,
    .frame_count = 0
};

static bool g_render_debug_enabled = false;
static bool g_virtual_scrolling_enabled = false;
static bool g_layout_cache_enabled = true;
static ImVec2 g_virtual_scroll_viewport = {0, 0};

// ============================================================================
// 渲染器状态管理
// ============================================================================

RendererState* get_renderer_state(void) {
    return &g_renderer_state;
}

void init_declarative_renderer_impl(void) {
    memset(&g_renderer_state, 0, sizeof(RendererState));
    g_renderer_state.dpi_scale = ImGui::GetIO().DisplayFramebufferScale.x;
    printf("Declarative renderer initialized with DPI scale: %.2f\n", g_renderer_state.dpi_scale);
}

void shutdown_declarative_renderer_impl(void) {
    if (g_renderer_state.current_tree) {
        release_widget_tree(g_renderer_state.current_tree);
        g_renderer_state.current_tree = nullptr;
    }
    printf("Declarative renderer shutdown. Total frames: %zu, Avg frame time: %.2f ms\n",
           g_renderer_state.frame_count,
           g_renderer_state.avg_frame_time);
}

// ============================================================================
// Widget渲染函数
// ============================================================================

void render_widget(Widget* widget, BuildContext* context) {
    if (!widget) {
        printf("Error: Attempted to render null widget\n");
        return;
    }
    
    // 检查是否需要跳过渲染（性能优化）
    if (should_skip_render(widget)) {
        g_renderer_state.skipped_widgets++;
        return;
    }
    
    g_renderer_state.rendered_widgets++;
    
    switch (widget->type) {
        case WIDGET_TYPE_TEXT:
            render_text_widget(widget, context);
            break;
        case WIDGET_TYPE_BUTTON:
            render_button_widget(widget, context);
            break;
        case WIDGET_TYPE_CHECKBOX:
            render_checkbox_widget(widget, context);
            break;
        case WIDGET_TYPE_SLIDER:
            render_slider_widget(widget, context);
            break;
        case WIDGET_TYPE_CONTAINER:
            render_container_widget(widget, context);
            break;
        case WIDGET_TYPE_WINDOW:
            render_window_widget(widget, context);
            break;
        default:
            printf("Warning: Unknown widget type: %d\n", widget->type);
            break;
    }
}

void render_widget_tree(WidgetTree* tree) {
    if (!tree) {
        printf("Error: Attempted to render null widget tree\n");
        return;
    }
    
    g_renderer_state.current_tree = tree;
    g_renderer_state.rendered_widgets = 0;
    g_renderer_state.skipped_widgets = 0;
    
    if (g_render_debug_enabled) {
        printf("=== Starting frame %zu ===\n", g_renderer_state.frame_count);
    }
    
    // 首先构建Widget树（如果需要）
    if (tree->needs_rebuild) {
        build_widget_tree(tree);
    }
    
    // 执行布局
    if (tree->needs_layout) {
        layout_widget_tree(tree);
    }
    
    // 渲染
    render_widget(tree->root_widget, tree->build_context);
    
    // 标记绘制完成
    tree->needs_paint = false;
    
    if (g_render_debug_enabled) {
        printf("=== Frame %zu complete ===\n", g_renderer_state.frame_count);
        printf("Rendered: %zu, Skipped: %zu widgets\n",
               g_renderer_state.rendered_widgets,
               g_renderer_state.skipped_widgets);
    }
}

// ============================================================================
// 具体Widget类型的渲染实现
// ============================================================================

void render_text_widget(Widget* widget, BuildContext* context) {
    Property* text_prop = widget_get_property(widget, "text");
    if (!text_prop) {
        printf("Error: Text widget missing 'text' property\n");
        return;
    }
    
    const char* text = get_property_string(text_prop);
    if (g_render_debug_enabled) {
        printf("[Text] %s\n", text ? text : "(null)");
    }
    
    ImGui::Text("%s", text ? text : "");
}

void render_button_widget(Widget* widget, BuildContext* context) {
    Property* label_prop = widget_get_property(widget, "label");
    if (!label_prop) {
        printf("Error: Button widget missing 'label' property\n");
        return;
    }
    
    const char* label = get_property_string(label_prop);
    bool clicked = ImGui::Button(label ? label : "Button");
    
    if (clicked) {
        Property* callback_prop = widget_get_property(widget, "on_click");
        if (callback_prop && callback_prop->type == PROP_TYPE_CALLBACK) {
            EventHandlerFn callback = callback_prop->value.callback;
            if (callback) {
                callback(nullptr);
            }
        }
    }
    
    if (g_render_debug_enabled) {
        printf("[Button] %s - Clicked: %s\n", label ? label : "", clicked ? "Yes" : "No");
    }
}

void render_checkbox_widget(Widget* widget, BuildContext* context) {
    Property* label_prop = widget_get_property(widget, "label");
    Property* state_prop = widget_get_property(widget, "state");
    
    if (!label_prop || !state_prop) {
        printf("Error: Checkbox widget missing required properties\n");
        return;
    }
    
    const char* label = get_property_string(label_prop);
    ReactiveState* state = (ReactiveState*)state_prop->value.widget_value;
    
    if (!state) {
        printf("Error: Checkbox widget has null state\n");
        return;
    }
    
    bool current_value = get_state_bool(state);
    bool changed = ImGui::Checkbox(label ? label : "", &current_value);
    
    if (changed) {
        update_state_from_bool(state, current_value);
    }
    
    if (g_render_debug_enabled) {
        printf("[Checkbox] %s - Value: %s, Changed: %s\n",
               label ? label : "",
               current_value ? "true" : "false",
               changed ? "Yes" : "No");
    }
}

void render_slider_widget(Widget* widget, BuildContext* context) {
    Property* label_prop = widget_get_property(widget, "label");
    Property* state_prop = widget_get_property(widget, "state");
    Property* min_prop = widget_get_property(widget, "min");
    Property* max_prop = widget_get_property(widget, "max");
    
    if (!label_prop || !state_prop || !min_prop || !max_prop) {
        printf("Error: Slider widget missing required properties\n");
        return;
    }
    
    const char* label = get_property_string(label_prop);
    ReactiveState* state = (ReactiveState*)state_prop->value.widget_value;
    float min_val = get_property_float(min_prop);
    float max_val = get_property_float(max_prop);
    
    if (!state) {
        printf("Error: Slider widget has null state\n");
        return;
    }
    
    float current_value = get_state_float(state);
    bool changed = ImGui::SliderFloat(label ? label : "", &current_value, min_val, max_val);
    
    if (changed) {
        update_state_from_float(state, current_value);
    }
    
    if (g_render_debug_enabled) {
        printf("[Slider] %s - Value: %.2f [%.2f, %.2f], Changed: %s\n",
               label ? label : "",
               current_value, min_val, max_val,
               changed ? "Yes" : "No");
    }
}

void render_container_widget(Widget* widget, BuildContext* context) {
    if (g_render_debug_enabled) {
        printf("[Container] Rendering %zu children\n", widget->child_count);
    }
    
    // 渲染所有子组件
    for (size_t i = 0; i < widget->child_count; i++) {
        render_widget(widget->children[i], context);
    }
}

void render_window_widget(Widget* widget, BuildContext* context) {
    Property* title_prop = widget_get_property(widget, "title");
    if (!title_prop) {
        printf("Error: Window widget missing 'title' property\n");
        return;
    }
    
    const char* title = get_property_string(title_prop);
    bool* p_open = nullptr; // 可以从属性中获取关闭按钮状态
    
    bool window_open = true;
    if (ImGui::Begin(title ? title : "Window", p_open ? &window_open : nullptr)) {
        // 渲染子组件
        if (widget->child_count > 0) {
            render_widget(widget->children[0], context);
        }
    }
    ImGui::End();
    
    if (g_render_debug_enabled) {
        printf("[Window] %s - Open: %s\n", title ? title : "", window_open ? "Yes" : "No");
    }
}

// ============================================================================
// 布局和绘制钩子
// ============================================================================

void setup_render_object_hooks(RenderObject* render_obj) {
    if (!render_obj) return;
    
    render_obj->perform_layout = declarative_perform_layout;
    render_obj->paint = declarative_paint;
}

void declarative_perform_layout(RenderObject* obj) {
    if (!obj) return;
    
    // 更新布局状态
    obj->needs_layout = false;
    
    // 递归布局子对象
    for (size_t i = 0; i < obj->child_count; i++) {
        if (obj->children[i]->needs_layout) {
            obj->children[i]->perform_layout(obj->children[i]);
        }
    }
}

void declarative_paint(RenderObject* obj) {
    if (!obj) return;
    
    // 更新绘制状态
    obj->needs_paint = false;
    
    // 渲染对应的Widget
    if (obj->element && obj->element->widget) {
        render_widget(obj->element->widget, obj->element->build_context);
    }
    
    // 递归绘制子对象
    for (size_t i = 0; i < obj->child_count; i++) {
        if (obj->children[i]->needs_paint) {
            obj->children[i]->paint(obj->children[i]);
        }
    }
}

// ============================================================================
// 状态到ImGui的桥接
// ============================================================================

float state_to_float(ReactiveState* state) {
    return get_state_float(state);
}

bool state_to_bool(ReactiveState* state) {
    return get_state_bool(state);
}

const char* state_to_string(ReactiveState* state) {
    return get_state_string(state);
}

void update_state_from_float(ReactiveState* state, float value) {
    set_state_float(state, value);
}

void update_state_from_bool(ReactiveState* state, bool value) {
    set_state_bool(state, value);
}

void update_state_from_string(ReactiveState* state, const char* value) {
    set_state_string(state, value);
}

// ============================================================================
// 响应式数据绑定
// ============================================================================

typedef struct {
    ReactiveState* state;
    StatefulElement* element;
} DataBinding;

static DataBinding* g_data_bindings = nullptr;
static size_t g_data_binding_count = 0;
static size_t g_data_binding_capacity = 0;

void data_binding_changed(ReactiveState* state, void* user_data) {
    // 状态改变时，标记对应的Element为dirty
    for (size_t i = 0; i < g_data_binding_count; i++) {
        if (g_data_bindings[i].state == state) {
            StatefulElement* element = g_data_bindings[i].element;
            if (element) {
                element->is_dirty = true;
                
                // 标记WidgetTree需要重建
                if (element->build_context && element->build_context->widget_tree) {
                    element->build_context->widget_tree->needs_rebuild = true;
                }
            }
            break;
        }
    }
}

void create_data_binding(ReactiveState* state, StatefulElement* element) {
    if (!state || !element) return;
    
    // 添加到绑定列表
    if (g_data_binding_count >= g_data_binding_capacity) {
        size_t new_capacity = g_data_binding_capacity == 0 ? 16 : g_data_binding_capacity * 2;
        DataBinding* new_bindings = (DataBinding*)realloc(
            g_data_bindings, new_capacity * sizeof(DataBinding)
        );
        if (!new_bindings) {
            printf("Error: Failed to allocate memory for data bindings\n");
            return;
        }
        g_data_bindings = new_bindings;
        g_data_binding_capacity = new_capacity;
    }
    
    g_data_bindings[g_data_binding_count].state = state;
    g_data_bindings[g_data_binding_count].element = element;
    g_data_binding_count++;
    
    // 添加状态监听器
    add_state_listener(state, data_binding_changed, nullptr);
    
    if (g_render_debug_enabled) {
        printf("Created data binding: state_id=%u, element_id=%u\n",
               state->state_id, element->element_id);
    }
}

void remove_data_binding(ReactiveState* state, StatefulElement* element) {
    for (size_t i = 0; i < g_data_binding_count; i++) {
        if (g_data_bindings[i].state == state && 
            g_data_bindings[i].element == element) {
            
            // 移除监听器
            remove_state_listener(state, data_binding_changed);
            
            // 移除绑定
            for (size_t j = i; j < g_data_binding_count - 1; j++) {
                g_data_bindings[j] = g_data_bindings[j + 1];
            }
            g_data_binding_count--;
            
            if (g_render_debug_enabled) {
                printf("Removed data binding: state_id=%u, element_id=%u\n",
                       state->state_id, element->element_id);
            }
            return;
        }
    }
}

// ============================================================================
// 性能优化
// ============================================================================

static bool is_widget_dirty(Widget* widget) {
    // 简化实现：总是返回true
    // 实际实现中应该检查widget及其属性是否发生变化
    return true;
}

void mark_widget_dirty(Widget* widget) {
    // 标记widget为需要更新
    // 简化实现：无操作
}

void batch_update_widgets(Widget** widgets, size_t count) {
    for (size_t i = 0; i < count; i++) {
        mark_widget_dirty(widgets[i]);
    }
}

bool should_skip_render(Widget* widget) {
    // 如果widget没有被标记为dirty，可以跳过渲染
    return !is_widget_dirty(widget);
}

// ============================================================================
// 调试和诊断
// ============================================================================

void enable_render_debug(bool enable) {
    g_render_debug_enabled = enable;
    printf("Render debug %s\n", enable ? "enabled" : "disabled");
}

void print_render_stats(void) {
    printf("=== Render Statistics ===\n");
    printf("Rendered widgets: %zu\n", g_renderer_state.rendered_widgets);
    printf("Skipped widgets: %zu\n", g_renderer_state.skipped_widgets);
    printf("Total frames: %zu\n", g_renderer_state.frame_count);
    printf("Average frame time: %.2f ms\n", g_renderer_state.avg_frame_time);
    printf("DPI scale: %.2f\n", g_renderer_state.dpi_scale);
}

void print_widget_tree(Widget* widget, int depth) {
    if (!widget) return;
    
    // 打印缩进
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // 打印widget类型
    const char* type_name = "Unknown";
    switch (widget->type) {
        case WIDGET_TYPE_TEXT: type_name = "Text"; break;
        case WIDGET_TYPE_BUTTON: type_name = "Button"; break;
        case WIDGET_TYPE_CHECKBOX: type_name = "Checkbox"; break;
        case WIDGET_TYPE_SLIDER: type_name = "Slider"; break;
        case WIDGET_TYPE_CONTAINER: type_name = "Container"; break;
        case WIDGET_TYPE_WINDOW: type_name = "Window"; break;
        default: break;
    }
    
    printf("[%s] ID: %u, Children: %zu\n", type_name, widget->widget_id, widget->child_count);
    
    // 递归打印子组件
    for (size_t i = 0; i < widget->child_count; i++) {
        print_widget_tree(widget->children[i], depth + 1);
    }
}

// ============================================================================
// 帧管理
// ============================================================================

void begin_declarative_frame(void) {
    // 重置统计
    g_renderer_state.rendered_widgets = 0;
    g_renderer_state.skipped_widgets = 0;
}

void end_declarative_frame(void) {
    // 更新平均帧时间
    g_renderer_state.frame_count++;
}

// ============================================================================
// 事件处理
// ============================================================================

void handle_declarative_events(void) {
    // 处理特定于声明式UI的事件
    // 简化实现：无操作
}

// ============================================================================
// 高级特性
// ============================================================================

void enable_virtual_scrolling(bool enable) {
    g_virtual_scrolling_enabled = enable;
    printf("Virtual scrolling %s\n", enable ? "enabled" : "disabled");
}

void set_virtual_scroll_viewport(float width, float height) {
    g_virtual_scroll_viewport.x = width;
    g_virtual_scroll_viewport.y = height;
}

void enable_layout_cache(bool enable) {
    g_layout_cache_enabled = enable;
    printf("Layout cache %s\n", enable ? "enabled" : "disabled");
}

void clear_layout_cache(void) {
    // 清除布局缓存
    // 简化实现：无操作
}
