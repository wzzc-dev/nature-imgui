#!/bin/bash
# Linux/macOS构建脚本

# 设置UTF-8编码
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

echo "========================================"
echo "nature-lang + Dear ImGui 构建脚本 (Unix)"
echo "========================================"

# WGPU配置
WGPU_DIR="external/wgpu-macos-aarch64-release"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    WGPU_DIR="external/wgpu-linux-x86_64-release"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    WGPU_DIR="external/wgpu-macos-aarch64-release"
fi

# 检查并安装依赖
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "检测到Linux系统"
    if ! command -v cmake &> /dev/null; then
        echo "正在安装CMake..."
        sudo apt-get update
        sudo apt-get install -y cmake
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "检测到macOS系统"
    if ! command -v brew &> /dev/null; then
        echo "请先安装Homebrew: https://brew.sh"
        exit 1
    fi
    if ! command -v cmake &> /dev/null; then
        echo "正在安装CMake..."
        brew install cmake
    fi
fi

# 从源码构建 SDL3 静态库
SDL_BUILD_DIR="external/SDL"
SDL_INSTALL_DIR="$HOME/sdl3-static"

echo "========================================"
echo "检查 SDL3 静态库..."
echo "========================================"

if [ -f "$SDL_INSTALL_DIR/lib/libSDL3.a" ]; then
    echo "[OK] SDL3 静态库已存在: $SDL_INSTALL_DIR/lib/libSDL3.a"
else
    echo "SDL3 静态库未找到，正在从源码构建..."
    echo ""

    if [ -d "$SDL_BUILD_DIR" ]; then
        echo "删除旧的 SDL 源码目录..."
        rm -rf "$SDL_BUILD_DIR"
    fi

    echo "正在克隆 SDL 仓库..."
    git clone https://github.com/libsdl-org/SDL.git "$SDL_BUILD_DIR"
    if [ $? -ne 0 ]; then
        echo "[FAIL] SDL 克隆失败"
        exit 1
    fi

    echo "切换到 SDL release-3.4.0..."
    cd "$SDL_BUILD_DIR"
    git checkout release-3.4.0
    if [ $? -ne 0 ]; then
        echo "[FAIL] SDL 切换分支失败"
        exit 1
    fi

    echo "创建构建目录..."
    mkdir -p build_static
    cd build_static

    echo "配置 CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DSDL_STATIC=ON \
        -DSDL_SHARED=OFF \
        -DCMAKE_INSTALL_PREFIX="$SDL_INSTALL_DIR"

    if [ $? -ne 0 ]; then
        echo "[FAIL] CMake 配置失败"
        exit 1
    fi

    echo "构建 SDL3 静态库..."
    make -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
    if [ $? -ne 0 ]; then
        echo "[FAIL] SDL3 构建失败"
        exit 1
    fi

    echo "安装 SDL3 静态库..."
    make install
    if [ $? -ne 0 ]; then
        echo "[FAIL] SDL3 安装失败"
        exit 1
    fi

    cd ../../..
    echo "[OK] SDL3 静态库构建完成"
fi

# 检查external目录是否存在，不存在则创建
if [ ! -d "external" ]; then
    echo "创建 external 目录..."
    mkdir external
fi

# 检查Dear ImGui是否存在
if [ ! -d "external/imgui" ]; then
    echo "Dear ImGui未找到，正在克隆..."
    cd external
    git clone https://github.com/ocornut/imgui.git
    cd ..
else
    echo "Dear ImGui已存在"
fi

# 检查WGPU是否存在
if [ ! -d "$WGPU_DIR" ]; then
    echo ""
    echo "========================================"
    echo "WGPU未找到！"
    echo "========================================"
    echo ""
    echo "请下载 WGPU-Native 库："
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macOS (Apple Silicon): https://github.com/gfx-rs/wgpu-native/releases"
        echo "下载 wgpu-macos-aarch64-release 并解压到: $WGPU_DIR"
    else
        echo "Linux (x86_64): https://github.com/gfx-rs/wgpu-native/releases"
        echo "下载 wgpu-linux-x86_64-release 并解压到: $WGPU_DIR"
    fi
    echo ""
    echo "目录结构应为："
    echo "  $WGPU_DIR/include/webgpu/webgpu.h"
    echo "  $WGPU_DIR/lib/libwgpu_native.a"
    echo ""
    exit 1
else
    echo "WGPU目录找到，正在验证..."

    # 验证WGPU文件
    WGPU_HEADER="$WGPU_DIR/include/webgpu/webgpu.h"
    WGPU_LIB_DIR="$WGPU_DIR/lib"

    if [ ! -f "$WGPU_HEADER" ]; then
        echo "[FAIL] webgpu.h 未找到: $WGPU_HEADER"
        echo "请检查WGPU目录结构"
        exit 1
    else
        echo "[OK] webgpu.h 已找到"
    fi

    if [[ "$OSTYPE" == "darwin"* ]]; then
        WGPU_LIB="$WGPU_LIB_DIR/libwgpu_native.a"
    else
        WGPU_LIB="$WGPU_LIB_DIR/libwgpu_native.so"
    fi

    if [ ! -f "$WGPU_LIB" ]; then
        echo "[FAIL] WGPU库文件未找到: $WGPU_LIB"
        echo "请检查WGPU目录结构"
        exit 1
    else
        echo "[OK] WGPU库文件已找到"
    fi

    echo "WGPU验证成功！"
fi

# 验证SDL3静态库
echo "========================================"
echo "验证 SDL3 静态库..."
echo "========================================"

if [ ! -f "$SDL_INSTALL_DIR/lib/libSDL3.a" ]; then
    echo "[FAIL] SDL3 静态库未找到: $SDL_INSTALL_DIR/lib/libSDL3.a"
    exit 1
else
    echo "[OK] SDL3 静态库已找到"
fi

if [ ! -d "$SDL_INSTALL_DIR/include" ]; then
    echo "[FAIL] SDL3 头文件目录未找到: $SDL_INSTALL_DIR/include"
    exit 1
else
    echo "[OK] SDL3 头文件目录已找到"
fi

# 创建构建目录
mkdir -p build
cd build

# 使用CMake配置项目
echo "配置CMake项目..."
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake配置失败！"
    exit 1
fi

# 构建项目
echo "构建项目..."
make -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
if [ $? -ne 0 ]; then
    echo "构建失败！"
    exit 1
fi

cd ..

TARGET_DIR="target"

# 检查并创建 target 目录
if [ ! -d $TARGET_DIR ]; then
    echo "创建 $TARGET_DIR 目录..."
    mkdir $TARGET_DIR
fi

# 拷贝运行时依赖库到 target 目录
echo "拷贝运行时库到 $TARGET_DIR 目录..."

# 拷贝WGPU库
if [[ "$OSTYPE" == "darwin"* ]]; then
    WGPU_LIB="$WGPU_DIR/lib/libwgpu_native.a"
else
    WGPU_LIB="$WGPU_DIR/lib/libwgpu_native.a"
fi

if [ -f "$WGPU_LIB" ]; then
    mkdir -p "$TARGET_DIR"
    cp "$WGPU_LIB" "$TARGET_DIR/"
    echo "[OK] WGPU库已拷贝到 target/"
else
    echo "[WARN] WGPU库未找到: $WGPU_LIB"
fi

# 拷贝imgui库
if [[ "$OSTYPE" == "darwin"* ]]; then
    IMGUI_LIB="build/libimgui.a"
else
    IMGUI_LIB="build/libimgui.a"
fi

if [ -f "$IMGUI_LIB" ]; then
    mkdir -p "$TARGET_DIR"
    cp "$IMGUI_LIB" "$TARGET_DIR/"
    echo "[OK] imgui库已拷贝到 target/"
else
    echo "[WARN] imgui库未找到: $IMGUI_LIB"
fi

# 复制库文件到 lib/darwin_arm64 目录
echo "========================================"
echo "复制库文件到 lib/darwin_arm64..."
echo "========================================"

# 创建目标目录
mkdir -p "lib/darwin_arm64"

# 复制 SDL3 静态库
if [ -f "$SDL_INSTALL_DIR/lib/libSDL3.a" ]; then
    cp "$SDL_INSTALL_DIR/lib/libSDL3.a" "lib/darwin_arm64/"
    echo "[OK] libSDL3.a 已复制到 lib/darwin_arm64/"
else
    echo "[WARN] libSDL3.a 未找到"
fi

# 复制 WGPU 库
if [ -f "$WGPU_DIR/lib/libwgpu_native.a" ]; then
    cp "$WGPU_DIR/lib/libwgpu_native.a" "lib/darwin_arm64/"
    echo "[OK] libwgpu_native.a 已复制到 lib/darwin_arm64/"
else
    echo "[WARN] libwgpu_native.a 未找到"
fi

# 复制 ImGui 库
if [ -f "build/libimgui.a" ]; then
    cp "build/libimgui.a" "lib/darwin_arm64/"
    echo "[OK] libimgui.a 已复制到 lib/darwin_arm64/"
else
    echo "[WARN] libimgui.a 未找到"
fi

echo "========================================"
echo "构建成功！"
echo "========================================"
echo "库文件位置:"
echo "  - build/libimgui.a"
echo "  - lib/darwin_arm64/libSDL3.a"
echo "  - lib/darwin_arm64/libwgpu_native.a"
echo "  - lib/darwin_arm64/libimgui.a"
echo "运行时库位置: target/"
echo ""
echo "库已准备就绪，可供 nature 使用"
echo ""
echo "使用方法："
echo "1. 在 nature 项目中链接 lib/darwin_arm64/libimgui.a"
echo "2. SDL3 和 WGPU 库在 lib/darwin_arm64/ 目录中"
echo "3. 这些库将在运行时自动加载"
echo "========================================"
