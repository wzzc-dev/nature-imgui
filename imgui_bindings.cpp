// imgui_bindings.cpp
// C++绑定文件，用于 nature-lang 调用Dear ImGui库 (WebGPU版本)

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static WGPUInstance             wgpu_instance = nullptr;
static WGPUDevice               wgpu_device = nullptr;
static WGPUSurface              wgpu_surface = nullptr;
static WGPUQueue                wgpu_queue = nullptr;
static WGPUSurfaceConfiguration wgpu_surface_configuration = {};
static int                      wgpu_surface_width = 1280;
static int                      wgpu_surface_height = 800;
static ImVec4                   g_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static WGPUSurfaceTexture       g_surface_texture = {};
static WGPUCommandEncoder       g_encoder = nullptr;
static WGPURenderPassEncoder    g_pass = nullptr;

// Forward declarations
static bool         InitWGPU(SDL_Window* window);
static WGPUSurface  CreateWGPUSurface(const WGPUInstance& instance, SDL_Window* window);

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

static void ResizeSurface(int width, int height)
{
    wgpu_surface_configuration.width  = wgpu_surface_width  = width;
    wgpu_surface_configuration.height = wgpu_surface_height = height;
    wgpuSurfaceConfigure( wgpu_surface, (WGPUSurfaceConfiguration*)&wgpu_surface_configuration );
}

// Setup ImGui navigation (keyboard and gamepad controls)
extern "C" ImGuiIO* setup_imgui_navigation(void)
{
    ImGuiIO* io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    return io;
}

extern "C" int sdl_init(void)
{
    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

extern "C" void sdl_terminate(SDL_Window* window) {
    // Cleanup
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    wgpuSurfaceUnconfigure(wgpu_surface);
    wgpuSurfaceRelease(wgpu_surface);
    wgpuQueueRelease(wgpu_queue);
    wgpuDeviceRelease(wgpu_device);
    wgpuInstanceRelease(wgpu_instance);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

extern "C" SDL_Window* create_window(const char* title, int width, int height) {
    // Update global width/height
    wgpu_surface_width = width;
    wgpu_surface_height = height;

    // Create SDL window graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow(title, width, height, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return nullptr;
    }

    // Initialize WGPU
    if (!InitWGPU(window)) {
        printf("Error: InitWGPU failed\n");
        SDL_DestroyWindow(window);
        return nullptr;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOther(window);

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = wgpu_device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_surface_configuration.format;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    return window;
}

extern "C" void destroy_window(SDL_Window* window) {
    if (window) {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        wgpuSurfaceUnconfigure(wgpu_surface);
        wgpuSurfaceRelease(wgpu_surface);
        wgpuQueueRelease(wgpu_queue);
        wgpuDeviceRelease(wgpu_device);
        wgpuInstanceRelease(wgpu_instance);

        SDL_DestroyWindow(window);
    }
}

extern "C" void imgui_init(SDL_Window* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForOther(window);

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = wgpu_device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_surface_configuration.format;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);
}

extern "C" void imgui_shutdown(void) {
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

extern "C" void imgui_new_frame(void) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

extern "C" void imgui_render(void) {
    ImGui::Render();
}

extern "C" bool imgui_process_event(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    return true;
}

extern "C" bool imgui_should_exit(SDL_Window* window) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            return true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            return true;
    }
    return false;
}

extern "C" void show_demo_window(bool* p_open) {
    ImGui::ShowDemoWindow(p_open);
}

extern "C" void begin_window(const char* name) {
    ImGui::Begin(name);
}

extern "C" void end_window(void) {
    ImGui::End();
}

extern "C" void text(const char* text) {
    ImGui::Text("%s", text);
}

extern "C" bool button(const char* label) {
    return ImGui::Button(label);
}

extern "C" void checkbox(const char* label, bool *v) {
    printf("checkbox: %s, %d\n", label, *v);
    ImGui::Checkbox(label, v);
}

extern "C" void slider_float(const char* label, float* v, float v_min, float v_max) {
    ImGui::SliderFloat(label, v, v_min, v_max);
}

extern "C" void color_edit3(const char* label) {
    ImGui::ColorEdit3(label,  (float*)&g_clear_color);
}

extern "C" void same_line(void) {
    ImGui::SameLine();
}

extern "C" void Text(const char* fmt, ...) {
    ImGui::Text(fmt);
}
extern "C" void set_clear_color(float r, float g, float b, float a) {
    g_clear_color = ImVec4(r, g, b, a);
}

extern "C" void begin_frame(SDL_Window* window) {
    // React to changes in screen size
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    if (width != wgpu_surface_width || height != wgpu_surface_height)
        ResizeSurface(width, height);

    // Check surface status
    wgpuSurfaceGetCurrentTexture(wgpu_surface, &g_surface_texture);
    if (ImGui_ImplWGPU_IsSurfaceStatusError(g_surface_texture.status)) {
        fprintf(stderr, "Unrecoverable Surface Texture status=%#.8x\n", g_surface_texture.status);
        abort();
    }
    if (ImGui_ImplWGPU_IsSurfaceStatusSubOptimal(g_surface_texture.status)) {
        if (g_surface_texture.texture)
            wgpuTextureRelease(g_surface_texture.texture);
        if (width > 0 && height > 0)
            ResizeSurface(width, height);
    }

    // Start Dear ImGui frame
    imgui_new_frame();
}

extern "C" void end_frame(void) {
    // Prepare render pass and submit
    WGPUTextureViewDescriptor view_desc = {};
    view_desc.format = wgpu_surface_configuration.format;
    view_desc.dimension = WGPUTextureViewDimension_2D;
    view_desc.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
    view_desc.arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
    view_desc.aspect = WGPUTextureAspect_All;

    WGPUTextureView texture_view = wgpuTextureCreateView(g_surface_texture.texture, &view_desc);

    WGPURenderPassColorAttachment color_attachments = {};
    color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attachments.loadOp = WGPULoadOp_Clear;
    color_attachments.storeOp = WGPUStoreOp_Store;
    color_attachments.clearValue = { g_clear_color.x * g_clear_color.w, g_clear_color.y * g_clear_color.w, g_clear_color.z * g_clear_color.w, g_clear_color.w };
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

    wgpuSurfacePresent(wgpu_surface);

    wgpuTextureViewRelease(texture_view);
    wgpuRenderPassEncoderRelease(pass);
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(cmd_buffer);
}
extern "C" void render() {
    ImGui::Render();
}

static bool InitWGPU(SDL_Window* window)
{
    WGPUTextureFormat preferred_fmt = WGPUTextureFormat_Undefined;  // acquired from SurfaceCapabilities

    // WGPU backend: Adapter and Device acquisition, Surface creation

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
