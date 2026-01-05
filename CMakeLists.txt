cmake_minimum_required(VERSION 3.22)
project(imgui-n C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SDL3
if(WIN32)
    # Windows下使用SDL3库
    set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3-3.4.0/x86_64-w64-mingw32")
    if(EXISTS "${SDL3_DIR}/lib/cmake/SDL3/SDL3Config.cmake")
        set(CMAKE_PREFIX_PATH "${SDL3_DIR}")
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
        message(STATUS "SDL3 found at: ${SDL3_DIR}")
    else()
        message(WARNING "SDL3 not found in external/SDL3. Please download SDL3 from https://github.com/libsdl-org/SDL/releases")
        message(WARNING "Expected structure: ${SDL3_DIR}/lib/cmake/SDL3/SDL3Config.cmake")
        set(SDL3_FOUND FALSE)
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux下使用pkg-config查找SDL3
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL3 sdl3)
    endif()
    if(NOT SDL3_FOUND)
        message(WARNING "SDL3 not found. Install with: sudo apt-get install libsdl3-dev (Ubuntu/Debian)")
    else()
        # 创建SDL3::SDL3别名目标
        add_library(SDL3::SDL3 INTERFACE IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${SDL3_LIBRARIES}"
        )
    endif()
elseif(APPLE)
    # macOS下使用从源码构建的SDL3静态库
    set(SDL3_INSTALL_DIR "$ENV{HOME}/sdl3-static")
    if(EXISTS "${SDL3_INSTALL_DIR}/lib/libSDL3.a")
        # 创建SDL3静态库目标
        add_library(SDL3::SDL3 STATIC IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            IMPORTED_LOCATION "${SDL3_INSTALL_DIR}/lib/libSDL3.a"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INSTALL_DIR}/include"
        )
        set(SDL3_FOUND TRUE)
        message(STATUS "SDL3 static library found at: ${SDL3_INSTALL_DIR}")
    else()
        message(WARNING "SDL3 not found. Run build.sh to build from source or install with: brew install sdl3")
    endif()
endif()

# 查找WGPU库
if(NOT EMSCRIPTEN)
    if(WIN32)
        set(WGPU_DIR "${CMAKE_SOURCE_DIR}/external/wgpu-windows-x86_64-gnu-release")
    elseif(APPLE)
        set(WGPU_DIR "${CMAKE_SOURCE_DIR}/external/wgpu-macos-aarch64-release")
    elseif(UNIX)
        set(WGPU_DIR "${CMAKE_SOURCE_DIR}/external/wgpu-linux-x86_64-release")
    endif()
    if(EXISTS "${WGPU_DIR}/include/webgpu/webgpu.h")
        set(IMGUI_WGPU_DIR "${WGPU_DIR}")
        message(STATUS "WGPU found at: ${WGPU_DIR}")
    else()
        message(WARNING "WGPU not found. Please download WGPU-Native from https://github.com/gfx-rs/wgpu-native/releases")
        message(WARNING "Expected structure: ${WGPU_DIR}/include/webgpu/webgpu.h")
    endif()
endif()

# 添加Dear ImGui库
# 假设Dear ImGui在imgui子目录中，如果没有，请修改路径
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

# 添加Dear ImGui源文件
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
)

# 添加C++绑定源文件
set(BINDING_SOURCES
    imgui_bindings.cpp
)

# 创建静态库
add_library(imgui STATIC ${IMGUI_SOURCES} ${BINDING_SOURCES})

# 包含目录
target_include_directories(imgui PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/lib
)

# 设置编译选项
if(MSVC)
    target_compile_options(imgui PRIVATE /W4)
else()
    target_compile_options(imgui PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 添加编译定义
target_compile_definitions(imgui PRIVATE "IMGUI_EXAMPLE_SDL3_WGPU")
if(IMGUI_WGPU_DIR)
    target_compile_definitions(imgui PRIVATE "IMGUI_IMPL_WEBGPU_BACKEND_WGPU")
endif()

# Apple平台特殊编译选项
if(APPLE)
    set_source_files_properties(${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/imgui_bindings.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/native/src/native_app.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/native/src/native_imgui.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

# 链接库（根据平台不同使用不同的链接方式）
if(WIN32)
    if(SDL3_FOUND AND IMGUI_WGPU_DIR)
        # WGPU-Native 库
        set(WGPU_NATIVE_LIB_DIR ${IMGUI_WGPU_DIR}/lib)
        find_library(WGPU_LIBRARY NAMES wgpu_native HINTS ${WGPU_NATIVE_LIB_DIR})

        set(OS_LIBRARIES d3dcompiler ws2_32 userenv bcrypt ntdll opengl32 Propsys RuntimeObject)
        target_link_libraries(imgui
            ${WGPU_LIBRARY}
            ${OS_LIBRARIES}
            SDL3::SDL3
        )
    else()
        message(FATAL_ERROR "SDL3 or WGPU not found")
    endif()
elseif(UNIX AND NOT APPLE)
    if(SDL3_FOUND AND IMGUI_WGPU_DIR)
        set(WGPU_NATIVE_LIB_DIR ${IMGUI_WGPU_DIR}/lib)
        find_library(WGPU_LIBRARY NAMES wgpu_native HINTS ${WGPU_NATIVE_LIB_DIR})

        set(OS_LIBRARIES "-lm -ldl")
        target_link_libraries(imgui
            ${WGPU_LIBRARY}
            ${OS_LIBRARIES}
            SDL3::SDL3
        )
    else()
        message(FATAL_ERROR "SDL3 or WGPU not found")
    endif()
elseif(APPLE)
    if(SDL3_FOUND AND IMGUI_WGPU_DIR)
        set_source_files_properties(${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
        set_source_files_properties(${CMAKE_SOURCE_DIR}/imgui_bindings.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
        set(WGPU_NATIVE_LIB_DIR ${IMGUI_WGPU_DIR}/lib)
        find_library(WGPU_LIBRARY NAMES wgpu_native HINTS ${WGPU_NATIVE_LIB_DIR})

        set(OS_LIBRARIES "-framework CoreFoundation -framework QuartzCore -framework Metal -framework MetalKit -framework Cocoa")
        target_link_libraries(imgui
            ${WGPU_LIBRARY}
            ${OS_LIBRARIES}
            SDL3::SDL3
        )
    else()
        message(FATAL_ERROR "SDL3 or WGPU not found")
    endif()
endif()

if(IMGUI_WGPU_DIR)
    target_include_directories(imgui PRIVATE ${IMGUI_WGPU_DIR}/include)
endif()

# 输出构建信息
message(STATUS "SDL3 found: ${SDL3_FOUND}")
message(STATUS "WGPU found: ${IMGUI_WGPU_DIR}")
message(STATUS "Building static library: libimgui.a (imgui.lib on Windows)")

