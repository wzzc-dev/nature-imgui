// advanced_declarative_demo.cpp
// 高级声明式UI示例（C++版本）

#include "declarative_ui.h"
#include "declarative_ui_bindings.h"
#include "imgui_bindings.h"
#include <stdio.h>

// ============================================================================
// 回调函数
// ============================================================================

static int g_button_click_count = 0;

void on_button_click(void* user_data) {
    g_button_click_count++;
    printf("Button clicked! Total clicks: %d\n", g_button_click_count);
}

void on_save_click(void* user_data) {
    printf("Settings saved!\n");
}

void on_reset_click(void* user_data) {
    printf("Settings reset!\n");
}

// ============================================================================
// 创建示例UI
// ============================================================================

Widget* create_settings_panel(void) {
    // 创建状态
    ReactiveState* enable_feature = create_state_bool(true);
    ReactiveState* volume = create_state_float(0.75f);
    ReactiveState* brightness = create_state_float(0.5f);
    ReactiveState* selected_option = create_state_int(0);
    ReactiveState* status_text = create_state_string("Ready");

    // 创建UI组件
    Widget* children[] = {
        // 标题
        create_text_widget("Settings Panel"),
        
        // 分隔符（使用空文本模拟）
        create_text_widget(""),
        
        // 功能开关
        create_checkbox_widget("Enable Feature", enable_feature),
        
        // 音量控制
        create_slider_widget("Volume", volume, 0.0f, 1.0f),
        
        // 亮度控制
        create_slider_widget("Brightness", brightness, 0.0f, 1.0f),
        
        // 分隔符
        create_text_widget(""),
        
        // 按钮
        create_container_widget((Widget*[]){
            create_button_widget("Save", on_save_click),
            create_button_widget("Reset", on_reset_click)
        }, 2),
        
        // 状态文本
        create_text_widget(""),
        create_text_widget("Status:")
    };

    return create_container_widget(children, sizeof(children) / sizeof(children[0]));
}

Widget* create_main_window(void) {
    // 创建主窗口内容
    Widget* content = create_container_widget((Widget*[]){
        create_text_widget("Welcome to Declarative UI Demo!"),
        create_text_widget(""),
        create_text_widget("This is a modern, declarative UI framework"),
        create_text_widget("built on top of Dear ImGui."),
        create_text_widget(""),
        create_button_widget("Click Me!", on_button_click)
    }, 6);

    return create_window_widget("Declarative UI Demo", content);
}

Widget* create_dashboard(void) {
    // 创建多个状态
    ReactiveState* cpu_usage = create_state_float(45.0f);
    ReactiveState* memory_usage = create_state_float(62.0f);
    ReactiveState* disk_usage = create_state_float(78.0f);
    ReactiveState* network_speed = create_state_float(0.0f);

    // 创建仪表板
    Widget* dashboard_widgets[] = {
        create_text_widget("System Dashboard"),
        create_text_widget(""),
        
        create_text_widget("CPU Usage:"),
        create_slider_widget("CPU", cpu_usage, 0.0f, 100.0f),
        
        create_text_widget("Memory Usage:"),
        create_slider_widget("Memory", memory_usage, 0.0f, 100.0f),
        
        create_text_widget("Disk Usage:"),
        create_slider_widget("Disk", disk_usage, 0.0f, 100.0f),
        
        create_text_widget("Network Speed:"),
        create_slider_widget("Network", network_speed, 0.0f, 100.0f),
        
        create_text_widget(""),
        create_button_widget("Refresh", nullptr)
    };

    return create_window_widget("Dashboard",
        create_container_widget(dashboard_widgets, sizeof(dashboard_widgets) / sizeof(dashboard_widgets[0]))
    );
}

// ============================================================================
// 主函数
// ============================================================================

int main(void) {
    printf("Starting Advanced Declarative UI Demo...\n");

    // 1. 初始化SDL
    if (sdl_init() != 0) {
        printf("Failed to initialize SDL\n");
        return 1;
    }
    printf("SDL initialized successfully!\n");

    // 2. 创建窗口
    SDL_Window* window = create_window("Advanced Declarative UI Demo", 1280, 800);
    if (!window) {
        printf("Failed to create window\n");
        sdl_terminate(window);
        return 1;
    }
    printf("Window created successfully!\n");

    // 3. 设置ImGui导航
    setup_imgui_navigation();

    // 4. 初始化声明式UI渲染器
    init_declarative_renderer();
    printf("Declarative renderer initialized!\n");

    // 5. 创建多个窗口
    bool show_main_window = true;
    bool show_settings = false;
    bool show_dashboard = false;

    ReactiveState* show_main_state = create_state_bool(true);
    ReactiveState* show_settings_state = create_state_bool(false);
    ReactiveState* show_dashboard_state = create_state_bool(false);

    Widget* main_window_widget = create_main_window();
    Widget* settings_window_widget = create_settings_panel();
    Widget* dashboard_widget = create_dashboard();

    WidgetTree* main_tree = decl_create_widget_tree(main_window_widget);
    WidgetTree* settings_tree = decl_create_widget_tree(settings_window_widget);
    WidgetTree* dashboard_tree = decl_create_widget_tree(dashboard_widget);

    // 6. 主循环
    printf("Starting main loop...\n");
    bool done = false;
    int frame_count = 0;

    while (!done) {
        done = imgui_should_exit(window);
        begin_frame(window);

        // 更新状态
        if (g_button_click_count > 0 && frame_count % 60 == 0) {
            // 每60帧重置一次点击计数（模拟动态更新）
        }
        frame_count++;

        // 渲染主窗口
        if (decl_get_state_bool(show_main_state)) {
            decl_render_widget_tree(main_tree);
        }

        // 渲染设置窗口
        if (decl_get_state_bool(show_settings_state)) {
            decl_render_widget_tree(settings_tree);
        }

        // 渲染仪表板窗口
        if (decl_get_state_bool(show_dashboard_state)) {
            decl_render_widget_tree(dashboard_tree);
        }

        // 添加演示菜单
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Windows")) {
                bool main_visible = decl_get_state_bool(show_main_state);
                bool settings_visible = decl_get_state_bool(show_settings_state);
                bool dashboard_visible = decl_get_state_bool(show_dashboard_state);

                if (ImGui::MenuItem("Main Window", nullptr, &main_visible)) {
                    decl_set_state_bool(show_main_state, main_visible);
                }
                if (ImGui::MenuItem("Settings", nullptr, &settings_visible)) {
                    decl_set_state_bool(show_settings_state, settings_visible);
                }
                if (ImGui::MenuItem("Dashboard", nullptr, &dashboard_visible)) {
                    decl_set_state_bool(show_dashboard_state, dashboard_visible);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    done = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                static bool debug_enabled = false;
                if (ImGui::MenuItem("Render Debug", nullptr, &debug_enabled)) {
                    decl_enable_render_debug(debug_enabled);
                }
                if (ImGui::MenuItem("Print Stats")) {
                    decl_print_render_stats();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 渲染ImGui
        imgui_render();
        end_frame();
    }

    // 7. 清理资源
    printf("Cleaning up...\n");
    
    decl_release_widget_tree(main_tree);
    decl_release_widget_tree(settings_tree);
    decl_release_widget_tree(dashboard_tree);
    
    decl_release_state(show_main_state);
    decl_release_state(show_settings_state);
    decl_release_state(show_dashboard_state);
    
    shutdown_declarative_renderer();
    sdl_terminate(window);
    
    printf("Demo finished. Total frames: %d, Button clicks: %d\n", frame_count, g_button_click_count);
    
    return 0;
}
