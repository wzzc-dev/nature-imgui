
## 技术栈

| 组件 | 说明 |
|------|------|
| **GUI 框架** | [Dear ImGui](https://github.com/ocornut/imgui) - 即时模式 GUI 库 |
| **渲染后端** | [WebGPU](https://www.w3.org/TR/webgpu/) - 现代图形 API |
| **WebGPU 实现** | [WGPU-Native](https://github.com/gfx-rs/wgpu-native) - WebGPU 的 Rust 实现 |
| **窗口系统** | [SDL3](https://github.com/libsdl-org/SDL) - 多媒体和输入处理库 |
| **构建系统** | CMake + Shell 脚本 |
| **目标语言** | [nature-lang](https://nature-lang.cn/) - 现代 AOT 编译语言 |


## 快速开始

### 前置要求

- **CMake** (≥ 3.22)
- **Git**
- **nature-lang** 编译器
- **C/C++ 编译器** (GCC/Clang/MSVC)

### 安装依赖

1. 克隆项目：
```bash
git clone <repository-url>
cd nature-imgui
```

2. 运行构建脚本自动下载和构建依赖：
```bash
chmod +x build.sh
./build.sh
```

构建脚本会自动：
- 从源码构建 SDL3 静态库
- 下载 Dear ImGui
- 检查 WGPU-Native 库
- 编译 C++ 绑定层
- 生成静态库文件

### 手动安装 WGPU-Native

如果构建脚本提示缺少 WGPU-Native，请手动下载：

**macOS (Apple Silicon):**
```bash
# 下载 wgpu-macos-aarch64-release
# 从 https://github.com/gfx-rs/wgpu-native/releases
# 解压到 external/wgpu-macos-aarch64-release/
```

**Linux (x86_64):**
```bash
# 下载 wgpu-linux-x86_64-release
# 从 https://github.com/gfx-rs/wgpu-native/releases
# 解压到 external/wgpu-linux-x86_64-release/
```

**Windows:**
```bash
# 下载 wgpu-windows-x86_64-gnu-release
# 从 https://github.com/gfx-rs/wgpu-native/releases
# 解压到 external/wgpu-windows-x86_64-gnu-release/
```

### 编译示例

```bash
# macOS (使用 nature-lang 编译)
nature build --ldflags \
  '-framework Cocoa -framework Metal -framework QuartzCore \
   -framework CoreVideo -framework IOKit -framework CoreGraphics \
   -framework CoreFoundation -framework AVFoundation -lc++' \
  -o target/demo \
  examples/demo.n
```

## 使用示例

以下是一个完整的示例，展示如何使用 Nature ImGui 创建一个简单的 GUI 程序：

```nature-lang
import co
import fmt
import imgui

fn main() {
    // 1. 初始化 SDL
    if imgui.sdl_init() != 0 {
        fmt.printf("Failed to initialize SDL\n")
        return
    }

    // 2. 创建窗口
    var title = "Hello ImGui"
    anyptr window = imgui.create_window(title.to_cstr() as anyptr, 1280, 800)

    // 3. 设置导航
    anyptr io = imgui.setup_imgui_navigation()
    bool show_demo_window = true

    // 4. 主循环
    bool done = false
    for !done {
        done = imgui.imgui_should_exit(window)
        imgui.begin_frame(window)

        // 创建窗口
        imgui.begin_window("Hello World".to_cstr())
        imgui.text("Welcome to Nature ImGui!".to_cstr())

        // 添加控件
        imgui.checkbox("Show Demo Window".to_cstr(), &show_demo_window)

        if imgui.button("Click Me".to_cstr()) {
            fmt.printf("Button clicked!\n")
        }

        imgui.end_window()

        // 显示演示窗口
        if show_demo_window {
            imgui.show_demo_window(show_demo_window)
        }

        imgui.imgui_render()
        imgui.end_frame()
    }

    // 5. 清理资源
    imgui.sdl_terminate(window)
}
```

## API 文档

### 初始化函数

```nature-lang
// SDL 初始化
fn sdl_init(): i32

// 创建窗口
fn create_window(anyptr title, i32 width, i32 height): anyptr

// 设置 ImGui 导航
fn setup_imgui_navigation(): anyptr
```

### 渲染循环

```nature-lang
// 开始新帧
fn begin_frame(anyptr window)

// 渲染
fn imgui_render()

// 结束帧
fn end_frame()

// 检查是否应该退出
fn imgui_should_exit(anyptr window): bool
```

### 窗口和控件

```nature-lang
// 窗口管理
fn begin_window(libc.cstr name)
fn end_window()

// 文本
fn text(libc.cstr text)

// 按钮
fn button(libc.cstr label): bool

// 复选框
fn checkbox(libc.cstr label, rawptr<bool> v)

// 滑块
fn slider_float(libc.cstr label, rawptr<f32> v, f32 v_min, f32 v_max)

// 颜色编辑器
fn color_edit3(libc.cstr label)

// 演示窗口
fn show_demo_window(bool p_open)
```

### 清理函数

```nature-lang
// 清理资源
fn sdl_terminate(anyptr window)
```

### 布局工具

```nature-lang
// 同行显示
fn same_line()

// 设置清除颜色
fn set_clear_color(f32 r, f32 g, f32 b, f32 a)
```

## 构建系统

项目使用 CMake 构建系统，主要包含：

- **CMakeLists.txt** - 主构建配置
- **build.sh** - 自动化构建脚本（Linux/macOS）
- **package.toml** - nature-lang 包配置

### 编译选项

```bash
# CMake 配置
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=17

# 编译
cd build
make -j$(nproc)
```

## 跨平台支持

| 平台 | 架构 | 状态 |
|------|------|------|
| macOS | ARM64 (Apple Silicon) | ✅ 支持 |
| macOS | x86_64 | ⚠️ 需调整 |
| Linux | x86_64 | ✅ 支持 |
| Windows | x86_64 | ⚠️ 需调整 |

## 依赖说明

### Dear ImGui

- **版本**: master 分支
- **用途**: 即时模式 GUI 框架
- **许可证**: MIT
- **仓库**: https://github.com/ocornut/imgui

### SDL3

- **版本**: release-3.4.0
- **用途**: 窗口管理和输入处理
- **许可证**: zlib
- **仓库**: https://github.com/libsdl-org/SDL

### WGPU-Native

- **版本**: 根据发布版本
- **用途**: WebGPU 实现
- **许可证**: MIT/Apache 2.0
- **仓库**: https://github.com/gfx-rs/wgpu-native

## 开发指南

### 添加新的绑定函数

1. 在 `imgui_bindings.h` 中声明函数
2. 在 `imgui_bindings.cpp` 中实现函数
3. 在 `main.n` 中添加 nature-lang FFI 声明
4. 在 CMakeLists.txt 中确保包含新文件
5. 运行构建脚本更新静态库

### 调试

启用详细日志：

```cpp
// imgui_bindings.cpp
wgpuSetLogLevel(WGPULogLevel_Info);
```

## 常见问题

### Q: 构建时找不到 WGPU-Native？

A: 确保已下载正确平台的 WGPU-Native 库并放置在 `external/` 目录下。

### Q: macOS 上链接错误？

A: 确保添加了所有必要的 frameworks：
```bash
-framework Cocoa -framework Metal -framework QuartzCore ...
```

### Q: 窗口显示异常？

A: 检查 DPI 缩放设置，确保调用 `setup_imgui_navigation()`。

## 贡献指南

欢迎贡献！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 致谢

- [Dear ImGui](https://github.com/ocornut/imgui) - 优秀的即时模式 GUI 库
- [SDL](https://github.com/libsdl-org/SDL) - 强大的跨平台多媒体库
- [WGPU-Native](https://github.com/gfx-rs/wgpu-native) - WebGPU 的 Rust 实现
- [nature-lang](https://nature-lang.cn/) - 现代 AOT 编译语言

