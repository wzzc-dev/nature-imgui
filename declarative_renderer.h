// declarative_renderer.h
// 声明式UI到Dear ImGui的渲染桥接
#ifndef DECLARATIVE_RENDERER_H
#define DECLARATIVE_RENDERER_H

#include "declarative_ui.h"
#include "imgui.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 渲染器状态
// ============================================================================

typedef struct {
    // 当前正在渲染的WidgetTree
    WidgetTree* current_tree;
    
    // 渲染统计
    size_t rendered_widgets;
    size_t skipped_widgets;
    
    // 布局和绘制上下文
    ImVec2 cursor_pos;
    ImVec2 window_size;
    float item_width;
    
    // DPI缩放
    float dpi_scale;
    
    // 性能统计
    double last_frame_time;
    double avg_frame_time;
    size_t frame_count;
} RendererState;

// 获取全局渲染器状态
RendererState* get_renderer_state(void);

// 初始化渲染器
void init_declarative_renderer_impl(void);

// 关闭渲染器
void shutdown_declarative_renderer_impl(void);

// ============================================================================
// Widget渲染函数
// ============================================================================

// 渲染单个Widget
void render_widget(Widget* widget, BuildContext* context);

// 渲染WidgetTree
void render_widget_tree(WidgetTree* tree);

// ============================================================================
// 具体Widget类型的渲染实现
// ============================================================================

// 渲染Text Widget
void render_text_widget(Widget* widget, BuildContext* context);

// 渲染Button Widget
void render_button_widget(Widget* widget, BuildContext* context);

// 渲染Checkbox Widget
void render_checkbox_widget(Widget* widget, BuildContext* context);

// 渲染Slider Widget
void render_slider_widget(Widget* widget, BuildContext* context);

// 渲染Container Widget
void render_container_widget(Widget* widget, BuildContext* context);

// 渲染Window Widget
void render_window_widget(Widget* widget, BuildContext* context);

// ============================================================================
// 布局和绘制钩子
// ============================================================================

// 为RenderObject设置布局和绘制钩子
void setup_render_object_hooks(RenderObject* render_obj);

// 布局钩子实现
void declarative_perform_layout(RenderObject* obj);

// 绘制钩子实现
void declarative_paint(RenderObject* obj);

// ============================================================================
// 状态到ImGui的桥接
// ============================================================================

// 将ReactiveState值转换为ImGui参数
float state_to_float(ReactiveState* state);
bool state_to_bool(ReactiveState* state);
const char* state_to_string(ReactiveState* state);

// 更新ReactiveState从ImGui
void update_state_from_float(ReactiveState* state, float value);
void update_state_from_bool(ReactiveState* state, bool value);
void update_state_from_string(ReactiveState* state, const char* value);

// ============================================================================
// 响应式数据绑定
// ============================================================================

// 创建数据绑定监听器
void create_data_binding(ReactiveState* state, StatefulElement* element);

// 移除数据绑定
void remove_data_binding(ReactiveState* state, StatefulElement* element);

// ============================================================================
// 性能优化
// ============================================================================

// 标记Widget为需要更新
void mark_widget_dirty(Widget* widget);

// 批量更新Widgets
void batch_update_widgets(Widget** widgets, size_t count);

// 跳过未改变的Widget渲染
bool should_skip_render(Widget* widget);

// ============================================================================
// 调试和诊断
// ============================================================================

// 启用渲染调试
void enable_render_debug(bool enable);

// 打印渲染统计信息
void print_render_stats(void);

// 打印Widget树结构
void print_widget_tree(Widget* widget, int depth);

// ============================================================================
// 帧管理
// ============================================================================

// 开始新帧
void begin_declarative_frame(void);

// 结束帧
void end_declarative_frame(void);

// ============================================================================
// 事件处理
// ============================================================================

// 处理ImGui事件到声明式UI
void handle_declarative_events(void);

// ============================================================================
// 高级特性
// ============================================================================

// 启用虚拟滚动（优化长列表渲染）
void enable_virtual_scrolling(bool enable);

// 设置虚拟滚动视口大小
void set_virtual_scroll_viewport(float width, float height);

// 启用布局缓存
void enable_layout_cache(bool enable);

// 清除布局缓存
void clear_layout_cache(void);

#ifdef __cplusplus
}
#endif

#endif // DECLARATIVE_RENDERER_H
