// declarative_ui_bindings.h
// 声明式UI的FFI绑定层（用于nature-lang调用）
#ifndef DECLARATIVE_UI_BINDINGS_H
#define DECLARATIVE_UI_BINDINGS_H

#include "declarative_ui.h"
#include "declarative_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 初始化和清理
// ============================================================================

void init_declarative_renderer(void);
void shutdown_declarative_renderer(void);

// ============================================================================
// Widget创建函数
// ============================================================================

Widget* decl_create_text_widget(const char* text);
Widget* decl_create_button_widget(const char* label, void (*callback)(void*));
Widget* decl_create_checkbox_widget(const char* label, ReactiveState* state);
Widget* decl_create_slider_widget(const char* label, ReactiveState* state, float min, float max);
Widget* decl_create_container_widget(Widget** children, int count);
Widget* decl_create_window_widget(const char* title, Widget* child);

// ============================================================================
// 状态管理函数
// ============================================================================

ReactiveState* decl_create_state_bool(bool value);
ReactiveState* decl_create_state_float(float value);
ReactiveState* decl_create_state_int(int value);
ReactiveState* decl_create_state_string(const char* value);

bool decl_get_state_bool(ReactiveState* state);
float decl_get_state_float(ReactiveState* state);
int decl_get_state_int(ReactiveState* state);
const char* decl_get_state_string(ReactiveState* state);

void decl_set_state_bool(ReactiveState* state, bool value);
void decl_set_state_float(ReactiveState* state, float value);
void decl_set_state_int(ReactiveState* state, int value);
void decl_set_state_string(ReactiveState* state, const char* value);

void decl_release_state(ReactiveState* state);

// ============================================================================
// WidgetTree管理函数
// ============================================================================

WidgetTree* decl_create_widget_tree(Widget* root);
void decl_build_widget_tree(WidgetTree* tree);
void decl_update_widget_tree(WidgetTree* tree, Widget* new_root);
void decl_render_widget_tree(WidgetTree* tree);
void decl_release_widget_tree(WidgetTree* tree);

// ============================================================================
// 渲染函数
// ============================================================================

void render_declarative_ui(void);

// ============================================================================
// 内存管理函数
// ============================================================================

void decl_release_widget(Widget* widget);

// ============================================================================
// 调试函数
// ============================================================================

void decl_enable_render_debug(bool enable);
void decl_print_render_stats(void);
void decl_print_widget_tree(Widget* widget, int depth);

#ifdef __cplusplus
}
#endif

#endif // DECLARATIVE_UI_BINDINGS_H
