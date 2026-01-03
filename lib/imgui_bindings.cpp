// imgui_bindings.cpp
// C++绑定文件，用于Moonbit调用Dear ImGui库 (WebGPU版本)

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局变量
static SDL_Window* g_window = NULL;
static WGPUInstance g_instance = NULL;
static WGPUSurface g_surface = NULL;
static WGPUAdapter g_adapter = NULL;
static WGPUDevice g_device = NULL;
static WGPUQueue g_queue = NULL;
static WGPUSurfaceConfiguration g_surface_config = {};

static WGPUInstance             wgpu_instance = nullptr;
static WGPUDevice               wgpu_device = nullptr;
static WGPUSurface              wgpu_surface = nullptr;
static WGPUQueue                wgpu_queue = nullptr;
static WGPUSurfaceConfiguration wgpu_surface_configuration = {};
static int                      wgpu_surface_width = 1280;
static int                      wgpu_surface_height = 800;
// Forward declarations
static bool         InitWGPU(SDL_Window* window);
static WGPUSurface  CreateWGPUSurface(const WGPUInstance& instance, SDL_Window* window);

static int g_width = 1280;
static int g_height = 720;
static float g_slider_value = 50.0f;
static bool g_checkbox_value = true;
static bool g_should_close = false;

// WebGPU 请求适配器的回调
static void handle_request_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2) {
    (void)userdata2;  // 未使用的参数
    if (status == WGPURequestAdapterStatus_Success) {
        WGPUAdapter* extAdapter = (WGPUAdapter*)userdata1;
        *extAdapter = adapter;
    } else {
        printf("Request_adapter status=%#.8x message=%.*s\n", status, (int)message.length, message.data);
    }
}

// WebGPU 请求设备的回调
static void handle_request_device(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2) {
    (void)userdata2;  // 未使用的参数
    if (status == WGPURequestDeviceStatus_Success) {
        WGPUDevice* extDevice = (WGPUDevice*)userdata1;
        *extDevice = device;
    } else {
        printf("Request_device status=%#.8x message=%.*s\n", status, (int)message.length, message.data);
    }
}

// 创建 WGPU 适配器
static WGPUAdapter RequestAdapter(WGPUInstance& instance) {
    WGPURequestAdapterOptions adapter_options = {};

    WGPUAdapter local_adapter;
    WGPURequestAdapterCallbackInfo adapterCallbackInfo = {};
    adapterCallbackInfo.callback = handle_request_adapter;
    adapterCallbackInfo.userdata1 = &local_adapter;
    adapterCallbackInfo.userdata2 = nullptr;

    wgpuInstanceRequestAdapter(instance, &adapter_options, adapterCallbackInfo);
    return local_adapter;
}

// 创建 WGPU 设备
static WGPUDevice RequestDevice(WGPUAdapter& adapter) {
    WGPUDevice local_device;
    WGPURequestDeviceCallbackInfo deviceCallbackInfo = {};
    deviceCallbackInfo.callback = handle_request_device;
    deviceCallbackInfo.userdata1 = &local_device;
    deviceCallbackInfo.userdata2 = nullptr;
    wgpuAdapterRequestDevice(adapter, nullptr, deviceCallbackInfo);
    return local_device;
}

// Dear ImGui上下文管理
extern "C" void imgui_create_context() {
    ImGui::CreateContext();
}

extern "C" void imgui_destroy_context() {
    ImGui::DestroyContext();
}

extern "C" void* imgui_get_io() {
    return (void*)&ImGui::GetIO();
}

extern "C" void imgui_new_frame() {
    ImGui::NewFrame();
}

extern "C" void imgui_render() {
    ImGui::Render();
}

extern "C" void* imgui_get_draw_data() {
    return (void*)ImGui::GetDrawData();
}

extern "C" void imgui_style_colors_dark() {
    ImGui::StyleColorsDark();
}

// 窗口管理
extern "C" int imgui_begin(const char* title) {
    return ImGui::Begin(title, NULL, 0);
}

extern "C" void imgui_end() {
    ImGui::End();
}

// 基本控件
extern "C" int imgui_button(const char* label) {
    return ImGui::Button(label);
}

extern "C" int imgui_slider_float(const char* label, double v, double v_min, double v_max) {
    (void)v;  // 未使用的参数
    int result = ImGui::SliderFloat(label, &g_slider_value, (float)v_min, (float)v_max);
    return result;
}

extern "C" int imgui_checkbox(const char* label, int v) {
    (void)v;  // 未使用的参数
    return ImGui::Checkbox(label, &g_checkbox_value);
}

extern "C" void imgui_text(const char* text) {
    ImGui::Text("%s", text);
}

extern "C" void imgui_separator() {
    ImGui::Separator();
}

extern "C" void imgui_same_line() {
    ImGui::SameLine();
}

// 颜色编辑器
static float g_clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

extern "C" void imgui_color_edit3(const char* label, float* r, float* g, float* b) {
    // 使用全局颜色数组
    float color[3] = {g_clear_color[0], g_clear_color[1], g_clear_color[2]};
    ImGui::ColorEdit3(label, color);
    // 更新全局颜色
    g_clear_color[0] = color[0];
    g_clear_color[1] = color[1];
    g_clear_color[2] = color[2];
    // 输出颜色值（Moonbit 无法接收输出参数，所以只是更新全局值）
    if (r) *r = color[0];
    if (g) *g = color[1];
    if (b) *b = color[2];
}

// 演示窗口
extern "C" void imgui_show_demo_window(void) {
    static bool show_demo = true;
    ImGui::ShowDemoWindow(&show_demo);
}


static void ResizeSurface(int width, int height)
{
    wgpu_surface_configuration.width  = wgpu_surface_width  = width;
    wgpu_surface_configuration.height = wgpu_surface_height = height;
    wgpuSurfaceConfigure( wgpu_surface, (WGPUSurfaceConfiguration*)&wgpu_surface_configuration );
}

extern "C" int sdl_init(void) 
{
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE;
    g_window = SDL_CreateWindow("Dear ImGui SDL3+WebGPU example", wgpu_surface_width, wgpu_surface_height, window_flags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    // Initialize WGPU
    InitWGPU(g_window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOther(g_window);

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = wgpu_device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_surface_configuration.format;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will call AddFontDefault() to select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);
#endif

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(g_window))
                done = true;
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        // React to changes in screen size
        int width, height;
        SDL_GetWindowSize(g_window, &width, &height);
        if (width != wgpu_surface_width || height != wgpu_surface_height)
            ResizeSurface(width, height);

        // Check surface status for error. If texture is not optimal, try to reconfigure the surface.
        WGPUSurfaceTexture surface_texture;
        wgpuSurfaceGetCurrentTexture(wgpu_surface, &surface_texture);
        if (ImGui_ImplWGPU_IsSurfaceStatusError(surface_texture.status))
        {
            fprintf(stderr, "Unrecoverable Surface Texture status=%#.8x\n", surface_texture.status);
            abort();
        }
        if (ImGui_ImplWGPU_IsSurfaceStatusSubOptimal(surface_texture.status))
        {
            if (surface_texture.texture)
                wgpuTextureRelease(surface_texture.texture);
            if (width > 0 && height > 0)
                ResizeSurface(width, height);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);         // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();

        WGPUTextureViewDescriptor view_desc = {};
        view_desc.format = wgpu_surface_configuration.format;
        view_desc.dimension = WGPUTextureViewDimension_2D;
        view_desc.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
        view_desc.arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
        view_desc.aspect = WGPUTextureAspect_All;

        WGPUTextureView texture_view = wgpuTextureCreateView(surface_texture.texture, &view_desc);

        WGPURenderPassColorAttachment color_attachments = {};
        color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        color_attachments.loadOp = WGPULoadOp_Clear;
        color_attachments.storeOp = WGPUStoreOp_Store;
        color_attachments.clearValue = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        color_attachments.view = texture_view;

        WGPURenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &color_attachments;
        render_pass_desc.depthStencilAttachment = nullptr;

        WGPUCommandEncoderDescriptor enc_desc = {};
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpu_device, &enc_desc);

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
        wgpuRenderPassEncoderEnd(pass);

        WGPUCommandBufferDescriptor cmd_buffer_desc = {};
        WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
        wgpuQueueSubmit(wgpu_queue, 1, &cmd_buffer);

#ifndef __EMSCRIPTEN__
        wgpuSurfacePresent(wgpu_surface);
        // Tick needs to be called in Dawn to display validation errors
#if defined(IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
        wgpuDeviceTick(wgpu_device);
#endif
#endif
        wgpuTextureViewRelease(texture_view);
        wgpuRenderPassEncoderRelease(pass);
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(cmd_buffer);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    

    return 0;
}

// // SDL3初始化和管理
// extern "C" int sdl_init(void) {
//     if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
//     {
//         printf("Error: SDL_Init(): %s\n", SDL_GetError());
//         return 1;
//     }

//     // Create SDL window graphics context
//     // 创建窗口
//     g_window = SDL_CreateWindow("Dear ImGui + Moonbit", g_width, g_height, SDL_WINDOW_RESIZABLE);
//     if (g_window == nullptr)
//     {
//         printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
//         return 1;
//     }

//     // 创建 WGPU 实例
//     WGPUInstanceDescriptor instance_desc = {};
//     instance_desc.nextInChain = NULL;
//     g_instance = wgpuCreateInstance(&instance_desc);
//     if (!g_instance) {
//         printf("Failed to create WGPU instance\n");
//         SDL_DestroyWindow(g_window);
//         SDL_Quit();
//         return 0;
//     }

//     // 请求 WGPU 适配器
//     g_adapter = RequestAdapter(g_instance);
//     if (!g_adapter) {
//         printf("Failed to get WGPU adapter\n");
//         wgpuInstanceRelease(g_instance);
//         SDL_DestroyWindow(g_window);
//         SDL_Quit();
//         return 0;
//     }

//     // 请求 WGPU 设备
//     g_device = RequestDevice(g_adapter);
//     if (!g_device) {
//         printf("Failed to get WGPU device\n");
//         wgpuAdapterRelease(g_adapter);
//         wgpuInstanceRelease(g_instance);
//         SDL_DestroyWindow(g_window);
//         SDL_Quit();
//         return 0;
//     }

//     // 获取队列
//     g_queue = wgpuDeviceGetQueue(g_device);

//     // 创建 WGPU Surface (使用 ImGui 的辅助函数)
//     SDL_PropertiesID propertiesID = SDL_GetWindowProperties(g_window);

//     ImGui_ImplWGPU_CreateSurfaceInfo create_info = {};
//     create_info.Instance = g_instance;
// #define SDL_PLATFORM_MACOS 1
// #if defined(SDL_PLATFORM_WIN32)
//     {
//         create_info.System = "win32";
//         create_info.RawWindow = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
//         create_info.RawInstance = (void*)GetModuleHandle(NULL);
//         g_surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
//     }
// #elif defined(SDL_PLATFORM_MACOS)
//     {
//         create_info.System = "metal";
//         create_info.RawWindow = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
//         create_info.RawInstance = NULL;
//         g_surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
//     }
// #else
//     // Linux 支持 (X11 或 Wayland)
//     {
//         create_info.System = "";
//         void* display = SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
//         if (display) {
//             create_info.System = "x11";
//             create_info.RawWindow = (void*)SDL_GetNumberProperty(propertiesID, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
//             create_info.RawInstance = display;
//         } else {
//             create_info.System = "wayland";
//             create_info.RawWindow = SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
//             create_info.RawInstance = SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
//         }
//         if (create_info.RawWindow) {
//             g_surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
//         }
//     }
//     if (!g_surface) {
//         printf("Unsupported platform for WebGPU or failed to create surface\n");
//         wgpuQueueRelease(g_queue);
//         wgpuDeviceRelease(g_device);
//         wgpuAdapterRelease(g_adapter);
//         wgpuInstanceRelease(g_instance);
//         SDL_DestroyWindow(g_window);
//         SDL_Quit();
//         return 0;
//     }
// #endif

//     if (!g_surface) {
//         printf("Failed to create WGPU surface\n");
//         wgpuQueueRelease(g_queue);
//         wgpuDeviceRelease(g_device);
//         wgpuAdapterRelease(g_adapter);
//         wgpuInstanceRelease(g_instance);
//         SDL_DestroyWindow(g_window);
//         SDL_Quit();
//         return 0;
//     }

//     // 配置 Surface
//     WGPUSurfaceCapabilities surface_capabilities = {};
//     wgpuSurfaceGetCapabilities(g_surface, g_adapter, &surface_capabilities);

//     g_surface_config.device = g_device;
//     g_surface_config.format = surface_capabilities.formats[0];
//     g_surface_config.usage = WGPUTextureUsage_RenderAttachment;
//     g_surface_config.alphaMode = WGPUCompositeAlphaMode_Auto;
//     g_surface_config.presentMode = WGPUPresentMode_Fifo;
//     g_surface_config.width = g_width;
//     g_surface_config.height = g_height;

//     wgpuSurfaceConfigure(g_surface, &g_surface_config);

//     return 1;
// }

extern "C" void sdl_create_window(int width, int height, const char* title) {
    // SDL 窗口已在 sdl_init 中创建，这里只更新标题
    if (g_window) {
        SDL_SetWindowTitle(g_window, title);
    }
    g_width = width;
    g_height = height;
}

extern "C" int sdl_window_should_close(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            g_should_close = true;
        }
        ImGui_ImplSDL3_ProcessEvent(&event);
    }
    return g_should_close;
}

extern "C" void sdl_swap_buffers(void) {
    // WebGPU 使用 surface，不需要手动 swap buffers
}

extern "C" void sdl_terminate(void) {
    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // 清理 WGPU 资源
    if (wgpu_surface) {
        wgpuSurfaceUnconfigure(wgpu_surface);
        wgpuSurfaceRelease(wgpu_surface);
        wgpu_surface = NULL;
    }
    if (wgpu_queue) {
        wgpuQueueRelease(wgpu_queue);
        wgpu_queue = NULL;
    }
    if (g_device) {
        wgpuDeviceRelease(g_device);
        g_device = NULL;
    }
    if (g_adapter) {
        wgpuAdapterRelease(g_adapter);
        g_adapter = NULL;
    }
    if (wgpu_instance) {
        wgpuInstanceRelease(wgpu_instance);
        wgpu_instance = NULL;
    }

    // 清理 SDL3 资源
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_Quit();
}

// ImGui-SDL3-WebGPU初始化
extern "C" int imgui_sdl3_init() {
    printf("window_ptr\n");
    
    // 检查窗口是否已创建
    if (!g_window) {
        printf("Error: SDL window not initialized\n");
        return 0;
    }
    
    // 检查ImGui上下文是否已创建
    if (!ImGui::GetCurrentContext()) {
        printf("Error: ImGui context not created\n");
        return 0;
    }
    
    // 设置ImGui风格
    ImGuiIO& io = ImGui::GetIO();
    printf("io.ConfigFlags\n");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    printf("io.ConfigFlags set\n");
    
    // 初始化ImGui后端
    if (!ImGui_ImplSDL3_InitForOther(g_window)) {
        printf("Failed to initialize ImGui SDL3 backend\n");
        return 0;
    }
    printf("ImGui_ImplSDL3_InitForOther\n");

    // 检查设备是否已创建
    if (!g_device) {
        printf("Error: WGPU device not initialized\n");
        return 0;
    }

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = g_device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = g_surface_config.format;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;

    if (!ImGui_ImplWGPU_Init(&init_info)) {
        printf("Failed to initialize ImGui WebGPU backend\n");
        return 0;
    }
    printf("ImGui_ImplWGPU_Init\n");

    return 1;
}

extern "C" void imgui_sdl3_shutdown(void) {
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

extern "C" void imgui_sdl3_new_frame(void) {
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplWGPU_NewFrame();
}

// WebGPU 渲染
extern "C" void render_frame(void) {
    // 检查窗口大小变化
    int width, height;
    SDL_GetWindowSize(g_window, &width, &height);
    if (width != g_width || height != g_height) {
        g_width = width;
        g_height = height;
        g_surface_config.width = width;
        g_surface_config.height = height;
        wgpuSurfaceConfigure(g_surface, &g_surface_config);
    }

    // 获取当前帧的纹理
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(g_surface, &surface_texture);

    if (ImGui_ImplWGPU_IsSurfaceStatusSubOptimal(surface_texture.status)) {
        if (surface_texture.texture) {
            wgpuTextureRelease(surface_texture.texture);
        }
        if (width > 0 && height > 0) {
            g_surface_config.width = width;
            g_surface_config.height = height;
            wgpuSurfaceConfigure(g_surface, &g_surface_config);
        }
        return;
    }

    if (!surface_texture.texture) {
        printf("Failed to get next texture\n");
        return;
    }

    // 创建纹理视图
    WGPUTextureViewDescriptor view_desc = {};
    view_desc.format = g_surface_config.format;
    view_desc.dimension = WGPUTextureViewDimension_2D;
    view_desc.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
    view_desc.arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
    view_desc.aspect = WGPUTextureAspect_All;

    WGPUTextureView texture_view = wgpuTextureCreateView(surface_texture.texture, &view_desc);

    // 设置渲染通道
    WGPURenderPassColorAttachment color_attach = {};
    color_attach.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attach.view = texture_view;
    color_attach.resolveTarget = nullptr;
    color_attach.loadOp = WGPULoadOp_Clear;
    color_attach.storeOp = WGPUStoreOp_Store;
    // 使用全局颜色配置
    color_attach.clearValue = {
        g_clear_color[0] * g_clear_color[3],
        g_clear_color[1] * g_clear_color[3],
        g_clear_color[2] * g_clear_color[3],
        g_clear_color[3]
    };

    WGPURenderPassDescriptor render_pass_desc = {};
    render_pass_desc.nextInChain = NULL;
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attach;
    render_pass_desc.depthStencilAttachment = nullptr;

    // 创建命令编码器
    WGPUCommandEncoderDescriptor encoder_desc = {};
    encoder_desc.nextInChain = NULL;
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(g_device, &encoder_desc);

    WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);

    // 渲染 ImGui
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass);

    wgpuRenderPassEncoderEnd(render_pass);

    // 提交命令
    WGPUCommandBufferDescriptor cmd_buf_desc = {};
    cmd_buf_desc.nextInChain = NULL;
    WGPUCommandBuffer cmd_buf = wgpuCommandEncoderFinish(encoder, &cmd_buf_desc);
    wgpuQueueSubmit(g_queue, 1, &cmd_buf);

    // 呈现
    wgpuSurfacePresent(g_surface);

    // 清理资源
    wgpuTextureViewRelease(texture_view);
    wgpuRenderPassEncoderRelease(render_pass);
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(cmd_buf);
    wgpuTextureRelease(surface_texture.texture);
}

static bool InitWGPU(SDL_Window* window)
{
    WGPUTextureFormat preferred_fmt = WGPUTextureFormat_Undefined;  // acquired from SurfaceCapabilities

    // Google DAWN backend: Adapter and Device acquisition, Surface creation
#if defined(IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
    wgpu::InstanceDescriptor instance_desc = {};
    static constexpr wgpu::InstanceFeatureName timedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    instance_desc.requiredFeatureCount = 1;
    instance_desc.requiredFeatures = &timedWaitAny;
    wgpu::Instance instance = wgpu::CreateInstance(&instance_desc);

    wgpu::Adapter adapter = RequestAdapter(instance);
    ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter.Get());

    wgpu_device = RequestDevice(instance, adapter);

    // Create the surface.
#ifdef __EMSCRIPTEN__
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvas_desc = {};
    canvas_desc.selector = "#canvas";

    wgpu::SurfaceDescriptor surface_desc = {};
    surface_desc.nextInChain = &canvas_desc;
    wgpu::Surface surface = instance.CreateSurface(&surface_desc);
#else
    wgpu::Surface surface = CreateWGPUSurface(instance.Get(), window);
#endif
    if (!surface)
        return false;

    // Moving Dawn objects into WGPU handles
    wgpu_instance = instance.MoveToCHandle();
    wgpu_surface = surface.MoveToCHandle();

    WGPUSurfaceCapabilities surface_capabilities = {};
    wgpuSurfaceGetCapabilities(wgpu_surface, adapter.Get(), &surface_capabilities);

    preferred_fmt = surface_capabilities.formats[0];

    // WGPU backend: Adapter and Device acquisition, Surface creation
#elif defined(IMGUI_IMPL_WEBGPU_BACKEND_WGPU)
    wgpu_instance = wgpuCreateInstance(nullptr);

#ifdef __EMSCRIPTEN__
    getAdapterAndDeviceViaJS();

    wgpu_device = emscripten_webgpu_get_device();
    IM_ASSERT(wgpu_device != nullptr && "Error creating the Device");

    WGPUSurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
    html_surface_desc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    html_surface_desc.selector = "#canvas";

    WGPUSurfaceDescriptor surface_desc = {};
    surface_desc.nextInChain = &html_surface_desc.chain;

    // Create the surface.
    wgpu_surface = wgpuInstanceCreateSurface(wgpu_instance, &surface_desc);
    preferred_fmt = wgpuSurfaceGetPreferredFormat(wgpu_surface, {} /* adapter */);
#else // __EMSCRIPTEN__
    wgpuSetLogCallback(
        [](WGPULogLevel level, WGPUStringView msg, void* userdata) { fprintf(stderr, "%s: %.*s\n", ImGui_ImplWGPU_GetLogLevelName(level), (int)msg.length, msg.data); }, nullptr
    );
    wgpuSetLogLevel(WGPULogLevel_Warn);

    WGPUAdapter adapter = RequestAdapter(wgpu_instance);
    ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter);

    wgpu_device = RequestDevice(adapter);

    // Create the surface.
    wgpu_surface = CreateWGPUSurface(wgpu_instance, window);
    if (!wgpu_surface)
        return false;

    WGPUSurfaceCapabilities surface_capabilities = {};
    wgpuSurfaceGetCapabilities(wgpu_surface, adapter, &surface_capabilities);

    preferred_fmt = surface_capabilities.formats[0];
#endif // __EMSCRIPTEN__
#endif // IMGUI_IMPL_WEBGPU_BACKEND_WGPU

    wgpu_surface_configuration.presentMode = WGPUPresentMode_Fifo;
    wgpu_surface_configuration.alphaMode = WGPUCompositeAlphaMode_Auto;
    wgpu_surface_configuration.usage = WGPUTextureUsage_RenderAttachment;
    wgpu_surface_configuration.width = wgpu_surface_width;
    wgpu_surface_configuration.height = wgpu_surface_height;
    wgpu_surface_configuration.device = wgpu_device;
    wgpu_surface_configuration.format = preferred_fmt;

    wgpuSurfaceConfigure(wgpu_surface, &wgpu_surface_configuration);
    wgpu_queue = wgpuDeviceGetQueue(wgpu_device);

    return true;
}

// SDL3 helper to create a WebGPU surface (exclusively!) for Native/Desktop applications: available only together with WebGPU/WGPU backend
// As of today (2025/10) there is no "official" support in SDL3 to create a surface for WebGPU backend
// This stub uses "low level" SDL3 calls to acquire information from a specific Window Manager.
// Currently supported platforms: Windows / Linux (X11 and Wayland) / MacOS. Not necessary nor available with EMSCRIPTEN.
#ifndef __EMSCRIPTEN__

#if defined(SDL_PLATFORM_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

static WGPUSurface CreateWGPUSurface(const WGPUInstance& instance, SDL_Window* window)
{
    SDL_PropertiesID propertiesID = SDL_GetWindowProperties(window);

    ImGui_ImplWGPU_CreateSurfaceInfo create_info = {};
    create_info.Instance = instance;
#if defined(SDL_PLATFORM_MACOS)
    {
        create_info.System = "cocoa";
        create_info.RawWindow = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
        return ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
    }
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        create_info.System = "wayland";
        create_info.RawDisplay = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        create_info.RawSurface = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        return ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
    }
    else if (!SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11"))
    {
        create_info.System = "x11";
        create_info.RawWindow = (void*)SDL_GetNumberProperty(propertiesID, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        create_info.RawDisplay = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        return ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
    }
#elif defined(SDL_PLATFORM_WIN32)
    {
        create_info.System = "win32";
        create_info.RawWindow = (void*)SDL_GetPointerProperty(propertiesID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        create_info.RawInstance = (void*)::GetModuleHandle(NULL);
        return ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_info);
    }
#else
#error "Unsupported WebGPU native platform!"
#endif
    return nullptr;
}
#endif // #ifndef __EMSCRIPTEN__
