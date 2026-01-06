# Agent Development Guide

本指南为 AI 助手提供关于 **Nature ImGui** 项目的全面信息，以便更好地理解、维护和扩展该代码库。

## 项目概述

**Nature ImGui** 是一个将 Dear ImGui 图形用户界面库绑定到 nature-lang 语言的 FFI（Foreign Function Interface）库。项目使用 WebGPU 作为渲染后端，SDL3 处理窗口和输入事件。

**关键特点：**
- 目标语言：**nature-lang**（现代 AOT 编译语言）
- 架构：**C++ FFI 绑定层** + **nature-lang 包装**
- 渲染：**WebGPU**（跨平台 GPU 加速）
- 窗口系统：**SDL3**
- 许可证：**MIT**
- 版本：**0.1.0**

## 技术架构

### 组件分层

```
┌─────────────────────────────────────────┐
│         nature-lang 应用层 (.n 文件)          │
│  (main.n, examples/demo.n, etc.)         │
└────────────────┬────────────────────────┘
                 │ nature-lang FFI
┌────────────────▼────────────────────────┐
│    C++ 绑定层 (imgui_bindings.cpp/h)     │
│  - WebGPU 初始化和管理                  │
│  - ImGui 状态管理                       │
│  - 事件处理                             │
└────────────────┬────────────────────────┘
                 │ C API
┌────────────────▼────────────────────────┐
│         Dear ImGui 库                    │
│  - imgui.cpp, imgui_draw.cpp, etc.      │
│  - imgui_impl_sdl3.cpp                   │
│  - imgui_impl_wgpu.cpp                   │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│      SDL3 + WGPU-Native                  │
│  - 窗口和输入管理 (SDL3)                 │
│  - WebGPU 渲染 (WGPU-Native)             │
└─────────────────────────────────────────┘
```

### 核心文件说明

#### 绑定层

| 文件 | 说明 | 关键内容 |
|------|------|----------|
| `imgui_bindings.h` | C 绑定头文件 | 声明所有 nature-lang 可调用的函数 |
| `imgui_bindings.cpp` | C++ 绑定实现 | WebGPU 初始化、ImGui 管理、渲染循环 |

#### 配置文件

| 文件 | 说明 | 关键配置 |
|------|------|----------|
| `package.toml` | nature-lang 包配置 | 库链接、依赖声明 |
| `CMakeLists.txt` | CMake 构建配置 | 编译选项、链接库、平台适配 |

#### 示例代码

| 文件 | 说明 |
|------|------|
| `examples/demo.n` | 完整功能示例（包含 Demo 窗口） |
| `examples/example.n` | 简化示例（基础控件） |
| `main.n` | nature-lang FFI 声明模块 |

## 关键实现细节

### WebGPU 初始化流程

**位置**: `imgui_bindings.cpp:355-413`

```cpp
static bool InitWGPU(SDL_Window* window)
```

关键步骤：
1. 创建 WGPU Instance
2. 请求 Adapter 和 Device
3. 创建 WGPU Surface
4. 配置 Surface（格式、尺寸、使用模式）
5. 获取 Command Queue

**平台适配**：
- **Windows**: 通过 SDL 获取 HWND 和 HINSTANCE
- **macOS**: 通过 SDL 获取 Cocoa window
- **Linux (X11)**: 通过 SDL 获取 X11 window 和 display
- **Linux (Wayland)**: 通过 SDL 获取 wayland surface

### 渲染循环流程

**位置**: `imgui_bindings.cpp:286-350`

每个渲染帧包含：

1. **begin_frame**:
   - 检查并调整 surface 尺寸
   - 获取当前 surface texture
   - 开始新 ImGui 帧

2. **ImGui 绘制**:
   - 调用 ImGui 控件函数
   - `imgui_render()` 准备绘制数据

3. **end_frame**:
   - 创建 RenderPass
   - 设置清除颜色
   - 执行 ImGui 绘制数据
   - 提交命令队列
   - 呈现 surface

### nature-lang FFI 绑定模式

**声明模式** (`main.n`):

```nature-lang
#linkid sdl_init
fn sdl_init(): i32

#linkid create_window
fn create_window(anyptr title, i32 width, i32 height): anyptr
```

**实现模式** (`imgui_bindings.cpp`):

```cpp
extern "C" int sdl_init(void) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}
```

**注意事项**：
- nature-lang 使用 `#linkid` 指定链接符号
- C 函数使用 `extern "C"` 避免名称修饰
- nature-lang 的 `anyptr` 对应 C 的 `void*`
- nature-lang 的 `libc.cstr` 对应 C 的 `const char*`

## 平台特定配置

### macOS

**编译选项** (CMakeLists.txt:116-121):
```cmake
if(APPLE)
    set_source_files_properties(imgui_impl_wgpu.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(imgui_bindings.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++")
endif()
```

**链接库**:
```bash
-framework Cocoa -framework Metal -framework QuartzCore \
-framework CoreVideo -framework IOKit -framework CoreGraphics \
-framework CoreFoundation -framework AVFoundation -framework CoreMedia \
-framework CoreAudio -framework AudioToolbox -framework CoreHaptics \
-framework GameController -framework ForceFeedback -framework Carbon \
-framework UniformTypeIdentifiers
```

**SDL3 静态库位置**: `~/sdl3-static/lib/libSDL3.a`

### Linux

**依赖**:
```bash
sudo apt-get install libsdl3-dev
```

**链接库**:
```bash
-lm -ldl
```

### Windows

**SDL3 目录**: `external/SDL3-3.4.0/x86_64-w64-mingw32`
**系统库**: `d3dcompiler ws2_32 userenv bcrypt ntdll opengl32 Propsys RuntimeObject`

## 常见开发任务

### 添加新的 ImGui 控件

1. **在 `imgui_bindings.h` 声明函数**:
```c
extern "C" void input_text(const char* label, char* buf, size_t buf_size);
```

2. **在 `imgui_bindings.cpp` 实现函数**:
```cpp
extern "C" void input_text(const char* label, char* buf, size_t buf_size) {
    ImGui::InputText(label, buf, buf_size);
}
```

3. **在 `main.n` 添加 FFI 声明**:
```nature-lang
#linkid input_text
fn input_text(libc.cstr label, rawptr<u8> buf, usize buf_size)
```

4. **重新构建**:
```bash
./build.sh
```

### 调试 WebGPU 问题

启用详细日志：
```cpp
// InitWGPU 函数中
wgpuSetLogLevel(WGPULogLevel_Info);
wgpuSetLogCallback([](WGPULogLevel level, WGPUStringView msg, void* userdata) {
    fprintf(stderr, "[%d] %.*s\n", level, (int)msg.length, msg.data);
}, nullptr);
```

### 处理 DPI 缩放

DPI 缩放在 `create_window` 中自动处理：
```cpp
float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
ImGuiStyle& style = ImGui::GetStyle();
style.ScaleAllSizes(main_scale);
style.FontScaleDpi = main_scale;
```

## 构建和测试

### 构建流程

```bash
# 1. 运行构建脚本
./build.sh

# 2. 脚本执行步骤：
#    - 检查/构建 SDL3 静态库
#    - 克隆/更新 Dear ImGui
#    - 验证 WGPU-Native
#    - CMake 配置
#    - make 编译
#    - 复制库文件到 lib/darwin_arm64/
```

### 编译示例程序

```bash
nature build --ldflags \
  '-framework Cocoa -framework Metal -framework QuartzCore \
   -framework CoreVideo -framework IOKit -framework CoreGraphics \
   -framework CoreFoundation -framework AVFoundation -lc++' \
  -o target/demo \
  examples/demo.n
```

### 运行示例

```bash
./target/demo
```

## 已知问题和限制

1. **Windows 支持**: 需要手动配置 SDL3 和 WGPU-Native 路径
2. **Linux 依赖**: 需要 pkg-config 和 libsdl3-dev
3. **静态库**: macOS 上的 SDL3 必须从源码构建
4. **WebGPU 兼容性**: 需要支持 WebGPU 的 GPU 和驱动

## 性能优化建议

1. **减少状态切换**: 合并相同类型的控件绘制
2. **批量提交**: 减少命令队列提交次数
3. **避免频繁重建窗口**: 保持窗口生命周期
4. **优化字体加载**: 预加载常用字体

## 测试策略

### 单元测试
目前 `tests/` 目录为空，建议添加：
- FFI 函数调用测试
- 内存泄漏检测
- 边界条件测试

### 集成测试
- 完整渲染流程测试
- 跨平台兼容性测试
- 性能基准测试

### 手动测试清单
- [ ] 窗口创建和销毁
- [ ] 所有控件功能
- [ ] 事件处理（键盘、鼠标、手柄）
- [ ] 窗口调整大小
- [ ] DPI 缩放
- [ ] 多窗口支持

## 代码规范

### C/C++ 代码
- 使用 `extern "C"` 导出函数
- 函数命名：snake_case
- 检查所有返回值
- 适当使用 RAII 管理资源

### nature-lang 代码
- 使用 FFI 声明时添加 `#linkid`
- C 字符串使用 `to_cstr()` 转换
- 指针类型使用 `anyptr` 或 `rawptr<T>`

## 未来扩展方向

1. **更多 ImGui 控件绑定**: 添加 Tree, List, Tab 等高级控件
2. **自定义着色器支持**: 允许用户自定义渲染
3. **多窗口支持**: 改进多窗口场景的处理
4. **文本输入增强**: 添加国际输入法支持
5. **主题系统**: 添加预定义主题和自定义主题支持

## 依赖版本

| 组件 | 版本 | 更新日期 |
|------|------|----------|
| Dear ImGui | master | - |
| SDL3 | 3.4.0 | - |
| WGPU-Native | latest | - |
| CMake | 3.22+ | - |
| nature-lang | 最新 | - |

## 参考资料

### 官方文档
- [Dear ImGui Documentation](https://github.com/ocornut/imgui)
- [SDL3 Wiki](https://wiki.libsdl.org/)
- [WebGPU Specification](https://www.w3.org/TR/webgpu/)
- [nature-lang Language Guide](https://nature-langlang.com/docs)

### 相关项目
- [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- [ImGui-SDL](https://github.com/Tyyppi77/imgui-sdl)

---

**注意**: 本文档仅供 AI 助手内部使用，不应作为用户文档。
