# Nature GUI Framework

基于Nature语言的声明式GUI框架，通过FFI绑定ImGui库，整合SDL3和wgpu作为底层支持，提供现代化的图形用户界面开发体验。

## 特性

### 核心功能
- **声明式UI组件库** - 50+UI组件，支持链式API和回调系统
- **现代渲染管线** - 基于WGPU的高性能GPU加速渲染
- **跨平台支持** - Windows、macOS、Linux统一API
- **类型安全FFI** - 清晰的C接口，完善的错误处理
- **分离式架构** - Native层、FFI层、运行时层清晰分离

### 技术栈
- **核心语言**：Nature
- **底层绑定**：C++ (FFI)
- **图形后端**：wgpu (WebGPU Native)
- **窗口/输入**：SDL3
- **即时模式GUI**：Dear ImGui

### 控件支持
- 基础控件：按钮、滑块、复选框、文本、输入框
- 高级控件：颜色选择器、树形控件、菜单、列表框
- 布局控件：容器、分隔线、缩进、间距
- 查询功能：点击、悬停、激活、聚焦状态

## 快速开始

### 1. 编译Native库

```bash
cd /home/chuang/nature-imgui
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 2. 运行示例

```bash
# 基础示例
cd /home/chuang/nature-imgui
nature run examples/demo.n

# 声明式示例
nature run examples/declarative_demo.n
```

### 3. 创建应用

```nature
import nature_gui
import nature_gui.runtime
import nature_gui.ui

fn main() {
    // 创建应用
    var config = nature_gui.runtime.ApplicationConfig{
        title: "My App",
        width: 800,
        height: 600
    }
    
    var app = nature_gui.runtime.Application.new(config)
    
    // 主循环
    app.run(fn(): void {
        // 构建UI
        if nature_gui.runtime.begin_window("Main Window", null, 0) {
            var btn = nature_gui.ui.Button.new(
                nature_gui.ui.ButtonConfig{label: "Click Me"}
            )
            btn.on_click = fn(): void {
                fmt.printf("Clicked!\n")
            }
            btn.render()
            
            nature_gui.runtime.end_window()
        }
    })
}
```
## 项目结构

```
nature-imgui/
├── native/                  # Native层（C++）
│   ├── include/
│   │   └── native_api.h     # FFI C接口定义
│   ├── src/
│   │   ├── native_app.cpp   # 应用生命周期管理
│   │   └── native_imgui.cpp # ImGui封装实现
│   └── DESIGN.md            # 接口设计文档
├── src/
│   ├── ffi/
│   │   └── native_ffi.n    # FFI外部函数声明
│   ├── runtime/
│   │   ├── application.n    # 应用生命周期
│   │   └── mod.n           # 模块导出
│   ├── ui/
│   │   ├── widgets.n       # UI组件库
│   │   └── mod.n           # 模块导出
│   └── mod.n                # 顶层模块导出
├── examples/
│   ├── demo.n               # 基础示例
│   └── declarative_demo.n   # 声明式示例
├── external/                # 第三方依赖
│   ├── imgui/               # Dear ImGui
│   └── wgpu-*/             # WGPU Native
├── CMakeLists.txt           # CMake构建配置
├── package.toml            # Nature包配置
├── README.md               # 本文档
└── IMPLEMENTATION.md        # 详细实现文档
```

## API文档

### 运行时API

#### Application.new(config: ApplicationConfig): Application
创建应用实例。

#### app.run(render_fn: RenderCallback): void
运行主循环，`render_fn`是每帧调用的UI构建函数。

#### app.begin_frame(): i32
开始新帧，返回错误码。

#### app.end_frame(): i32
结束帧并呈现，返回错误码。

#### app.should_close(): bool
检查是否应该关闭。

### 组件API

#### Button
```nature
var btn = Button.new(ButtonConfig{label: "Click Me"})
btn.on_click = fn(): void { /* ... */ }
btn.render()
```

#### Slider
```nature
var slider = Slider.new(SliderConfig{
    label: "Volume",
    value: 50.0f,
    min: 0.0f,
    max: 100.0f
})
slider.on_change = fn(value: f32): void { /* ... */ }
slider.render()
```

#### Checkbox
```nature
var checkbox = Checkbox.new(CheckboxConfig{
    label: "Enable",
    checked: true
})
checkbox.on_change = fn(checked: bool): void { /* ... */ }
checkbox.render()
```

#### InputText
```nature
var input = InputText.new(InputTextConfig{
    label: "Name",
    text: "Hello"
})
input.on_change = fn(text: string): void { /* ... */ }
input.render()
```

#### ColorPicker
```nature
var picker = ColorPicker.new(ColorPickerConfig{
    label: "Color",
    r: 1.0f,
    g: 0.5f,
    b: 0.5f
})
picker.on_change = fn(r, g, b, a: f32): void { /* ... */ }
picker.render()
```

## 声明式UI

使用组件链式API构建声明式UI：

```nature
var window = Container.new(ContainerConfig{title: "Settings"})

window.add(Button.new(ButtonConfig{label: "OK"})
    .on_click(fn(): void { /* OK */ }))

window.add(Slider.new(SliderConfig{
    label: "Volume",
    value: 50.0f,
    min: 0.0f,
    max: 100.0f
}).on_change(fn(v: f32): void {
    fmt.printf("Volume: %.1f\n", v)
}))

window.render()
```

## 文档

- [实现文档](IMPLEMENTATION.md) - 详细的架构设计和实现说明
- [设计文档](native/DESIGN.md) - FFI接口设计原则和规范

## 依赖

- SDL3 >= 3.0
- WGPU-Native
- Dear ImGui >= 1.92
- Nature Lang编译器

## 构建

### Linux
```bash
# 安装依赖
sudo apt-get install libsdl3-dev cmake build-essential

# 下载WGPU-Native
# 放置到 external/wgpu-linux-x86_64-release/

# 构建Native库
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### macOS
```bash
# 安装依赖
brew install sdl3 cmake

# 下载WGPU-Native
# 放置到 external/wgpu-macos-aarch64-release/

# 构建Native库
mkdir -p build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Windows
```bash
# 安装依赖
# 使用vcpkg安装SDL3和CMake
# 下载WGPU-Native放置到 external/wgpu-windows-x86_64-gnu-release/

# 构建Native库
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j
```

## 许可证

MIT

## 贡献

欢迎提交Issue和Pull Request！

## 致谢

- [Dear ImGui](https://github.com/ocornut/imgui) - 即时模式GUI库
- [SDL3](https://github.com/libsdl-org/SDL) - 跨平台多媒体库
- [wgpu-native](https://github.com/gfx-rs/wgpu-native) - WebGPU Native实现
- [Nature Lang](https://github.com/nature-lang/nature) - Nature编程语言

