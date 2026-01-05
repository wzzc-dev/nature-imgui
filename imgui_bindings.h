// imgui_bindings.h
// C绑定头文件，声明所有Moonbit调用的函数 (WebGPU版本)

#ifndef IMGUI_BINDINGS_H
#define IMGUI_BINDINGS_H

#ifdef __cplusplus
extern "C" {
#endif

// Dear ImGui上下文管理
void imgui_create_context(void);
void imgui_destroy_context(void);
void* imgui_get_io(void);
void imgui_new_frame(void);
void imgui_render(void);
void* imgui_get_draw_data(void);
void imgui_style_colors_dark(void);

// 窗口管理
int imgui_begin(const char* title);
void imgui_end(void);

// 基本控件
int imgui_button(const char* label);
int imgui_slider_float(const char* label, double v, double v_min, double v_max);
int imgui_checkbox(const char* label, int v);
void imgui_text(const char* text);
void imgui_separator(void);
void imgui_same_line(void);

// 颜色编辑器
void imgui_color_edit3(const char* label, float* r, float* g, float* b);

// 演示窗口
void imgui_show_demo_window(void);

// SDL3初始化和管理
int sdl_init(void);
void sdl_create_window(int width, int height, const char* title);
int sdl_window_should_close(void);
void sdl_swap_buffers(void);
void sdl_terminate(void);

// ImGui-SDL3-WebGPU初始化
int imgui_sdl3_init(void* window);
void imgui_sdl3_shutdown(void);
void imgui_sdl3_new_frame(void);

// WebGPU渲染
void render_frame(void);

void main_loop(void);

#ifdef __cplusplus
}
#endif

#endif // IMGUI_BINDINGS_H
