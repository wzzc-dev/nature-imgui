// imgui_bindings.h
// C绑定头文件，声明所有Moonbit调用的函数 (WebGPU版本)

#ifndef IMGUI_BINDINGS_H
#define IMGUI_BINDINGS_H

#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

int sdl_init(void);

void main_loop(SDL_Window*);
SDL_Window* create_window();

#ifdef __cplusplus
}
#endif

#endif // IMGUI_BINDINGS_H
