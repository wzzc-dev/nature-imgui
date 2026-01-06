// imgui_bindings.h
// C绑定头文件，声明所有Moonbit调用的函数 (WebGPU版本)

#ifndef IMGUI_BINDINGS_H
#define IMGUI_BINDINGS_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "imgui.h"

#ifdef __cplusplus
extern "C" {
#endif

// SDL 初始化和清理
int sdl_init(void);
void sdl_terminate(SDL_Window* window);

// 窗口创建和管理
SDL_Window* create_window(const char* title, int width, int height);
void destroy_window(SDL_Window* window);

ImGuiIO* setup_imgui_navigation(void);

// ImGui 上下文管理
void imgui_init(SDL_Window* window);
void imgui_shutdown(void);

// 渲染循环相关
void imgui_new_frame(void);
void imgui_render(void);

// 事件处理
bool imgui_process_event(SDL_Event* event);
bool imgui_should_exit(SDL_Window* window);

// 常用窗口和控件
void show_demo_window(bool* p_open);
void begin_window(const char* name);
void end_window(void);
void text(const char* text);
bool button(const char* label);
void checkbox(const char* label, bool v);
void slider_float(const char* label, float v, float v_min, float v_max);
void color_edit3(const char* label, float* col);
void same_line(void);

// 布局和样式
void set_clear_color(float r, float g, float b, float a);
void begin_frame(SDL_Window* window);
void end_frame(void);

// 主循环（保留用于简单示例）
void main_loop(SDL_Window* window);

#ifdef __cplusplus
}
#endif

#endif // IMGUI_BINDINGS_H
