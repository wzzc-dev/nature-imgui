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

    # 安装SDL3开发包
    if ! dpkg -l | grep -q libsdl3-dev; then
        echo "正在安装SDL3..."
        sudo apt-get install -y libsdl3-dev libgl1-mesa-dev
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "检测到macOS系统"
    if ! command -v brew &> /dev/null; then
        echo "请先安装Homebrew: https://brew.sh"
        exit 1
    fi
    brew install sdl3 cmake
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
    echo "  $WGPU_DIR/lib/libwgpu_native.dylib (macOS) 或 libwgpu_native.so (Linux)"
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
        WGPU_LIB="$WGPU_LIB_DIR/libwgpu_native.dylib"
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

# 验证SDL3
echo "正在验证SDL3..."
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if ! dpkg -l | grep -q libsdl3-dev; then
        echo "[FAIL] SDL3开发包未安装"
        echo "请运行: sudo apt-get install libsdl3-dev"
        exit 1
    else
        echo "[OK] SDL3开发包已安装"
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    if ! brew list sdl3 &> /dev/null; then
        echo "[FAIL] SDL3未通过Homebrew安装"
        echo "请运行: brew install sdl3"
        exit 1
    else
        echo "[OK] SDL3已通过Homebrew安装"
    fi
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
    WGPU_LIB="$WGPU_DIR/lib/libwgpu_native.dylib"
else
    WGPU_LIB="$WGPU_DIR/lib/libwgpu_native.so"
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
    IMGUI_LIB="build/libimgui.dylib"
else
    IMGUI_LIB="build/libimgui.so"
fi

if [ -f "$IMGUI_LIB" ]; then
    mkdir -p "$TARGET_DIR"
    cp "$IMGUI_LIB" "$TARGET_DIR/"
    echo "[OK] imgui库已拷贝到 target/"
else
    echo "[WARN] imgui库未找到: $IMGUI_LIB"
fi

# macOS下修复动态库加载路径
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "修复动态库加载路径 (macOS)..."

    # 先修复库文件的内部路径
    install_name_tool -id "@rpath/libimgui.dylib" "$TARGET_DIR/libimgui.dylib" 2>/dev/null
    install_name_tool -id "@rpath/libwgpu_native.dylib" "$TARGET_DIR/libwgpu_native.dylib" 2>/dev/null
    install_name_tool -change "@rpath/libwgpu_native.dylib" "@rpath/libwgpu_native.dylib" "$TARGET_DIR/libimgui.dylib" 2>/dev/null

    # 检查是否存在 nature 命令
    if command -v nature &> /dev/null; then
            echo "运行 nature 构建..."
            # 只构建main包，而不是整个项目
            nature build -o "$TARGET_DIR/main" main.n

            # nature 构建完成后，修复可执行文件的 rpath
            EXECUTABLE="$TARGET_DIR/main"
            if [ ! -f "$EXECUTABLE" ]; then
                EXECUTABLE="$TARGET_DIR/main.exe"
            fi

            if [ -f "$EXECUTABLE" ]; then
                # 添加 rpath，使程序可以从同一目录加载动态库
                install_name_tool -delete_rpath /usr/local/lib "$EXECUTABLE" 2>/dev/null || true
                install_name_tool -delete_rpath @executable_path "$EXECUTABLE" 2>/dev/null || true
                install_name_tool -add_rpath "@loader_path" "$EXECUTABLE" 2>/dev/null

                # 修改可执行文件对 imgui 库的引用
                install_name_tool -change "libimgui.dylib" "@rpath/libimgui.dylib" "$EXECUTABLE" 2>/dev/null

                echo "[OK] 可执行文件动态库路径已修复"
            fi
        else
            echo "未找到 nature 命令，跳过 nature 构建"
            echo "请手动运行 'nature build main.n' 后执行以下命令修复动态库路径："
            echo "  install_name_tool -add_rpath @loader_path $TARGET_DIR/main.exe"
            echo "  install_name_tool -change libimgui.dylib @rpath/libimgui.dylib $TARGET_DIR/main.exe"
        fi

    echo "[OK] 库文件动态库路径已修复"
fi

echo "========================================"
echo "构建成功！"
echo "========================================"
echo "库文件位置: build/libimgui.dylib (macOS) 或 build/libimgui.so (Linux)"
echo "运行时库位置: target/"
echo ""
echo "库已准备就绪，可供 nature 使用"
echo ""
echo "使用方法："
echo "1. 在 nature 项目中链接 build/libimgui.a"
echo "2. SDL3 和 WGPU 库在 target/ 目录中"
echo "3. 这些库将在运行时自动加载"
echo "========================================"
