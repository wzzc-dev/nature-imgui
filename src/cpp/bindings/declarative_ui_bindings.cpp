// declarative_ui_bindings.cpp
// 声明式UI的FFI绑定实现

#include "declarative_ui_bindings.h"
#include <stdio.h>

// ============================================================================
// 初始化和清理
// ============================================================================

extern "C" void init_declarative_renderer(void) {
    init_declarative_renderer_impl();
}

extern "C" void shutdown_declarative_renderer(void) {
    shutdown_declarative_renderer_impl();
}

// ============================================================================
// Widget创建函数
// ============================================================================

extern "C" Widget* decl_create_text_widget(const char* text) {
    return create_text_widget(text);
}

extern "C" Widget* decl_create_button_widget(const char* label, void (*callback)(void*)) {
    return create_button_widget(label, callback);
}

extern "C" Widget* decl_create_checkbox_widget(const char* label, ReactiveState* state) {
    return create_checkbox_widget(label, state);
}

extern "C" Widget* decl_create_slider_widget(const char* label, ReactiveState* state, float min, float max) {
    return create_slider_widget(label, state, min, max);
}

extern "C" Widget* decl_create_container_widget(Widget** children, int count) {
    return create_container_widget(children, count);
}

extern "C" Widget* decl_create_window_widget(const char* title, Widget* child) {
    return create_window_widget(title, child);
}

// ============================================================================
// 状态管理函数
// ============================================================================

extern "C" ReactiveState* decl_create_state_bool(bool value) {
    return create_state_bool(value);
}

extern "C" ReactiveState* decl_create_state_float(float value) {
    return create_state_float(value);
}

extern "C" ReactiveState* decl_create_state_int(int value) {
    return create_state_int(value);
}

extern "C" ReactiveState* decl_create_state_string(const char* value) {
    return create_state_string(value);
}

extern "C" bool decl_get_state_bool(ReactiveState* state) {
    return get_state_bool(state);
}

extern "C" float decl_get_state_float(ReactiveState* state) {
    return get_state_float(state);
}

extern "C" int decl_get_state_int(ReactiveState* state) {
    return get_state_int(state);
}

extern "C" const char* decl_get_state_string(ReactiveState* state) {
    return get_state_string(state);
}

extern "C" void decl_set_state_bool(ReactiveState* state, bool value) {
    set_state_bool(state, value);
}

extern "C" void decl_set_state_float(ReactiveState* state, float value) {
    set_state_float(state, value);
}

extern "C" void decl_set_state_int(ReactiveState* state, int value) {
    set_state_int(state, value);
}

extern "C" void decl_set_state_string(ReactiveState* state, const char* value) {
    set_state_string(state, value);
}

extern "C" void decl_release_state(ReactiveState* state) {
    release_state(state);
}

// ============================================================================
// WidgetTree管理函数
// ============================================================================

extern "C" WidgetTree* decl_create_widget_tree(Widget* root) {
    return create_widget_tree(root);
}

extern "C" void decl_build_widget_tree(WidgetTree* tree) {
    build_widget_tree(tree);
}

extern "C" void decl_update_widget_tree(WidgetTree* tree, Widget* new_root) {
    update_widget_tree(tree, new_root);
}

extern "C" void decl_render_widget_tree(WidgetTree* tree) {
    render_widget_tree(tree);
}

extern "C" void decl_release_widget_tree(WidgetTree* tree) {
    release_widget_tree(tree);
}

// ============================================================================
// 渲染函数
// ============================================================================

extern "C" void render_declarative_ui(void) {
    RendererState* state = get_renderer_state();
    if (state && state->current_tree) {
        render_widget_tree(state->current_tree);
    }
}

// ============================================================================
// 内存管理函数
// ============================================================================

extern "C" void decl_release_widget(Widget* widget) {
    release_widget(widget);
}

// ============================================================================
// 调试函数
// ============================================================================

extern "C" void decl_enable_render_debug(bool enable) {
    enable_render_debug(enable);
}

extern "C" void decl_print_render_stats(void) {
    print_render_stats();
}

extern "C" void decl_print_widget_tree(Widget* widget, int depth) {
    print_widget_tree(widget, depth);
}
