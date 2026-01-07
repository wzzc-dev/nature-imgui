// simple_declarative_demo.cpp
// 简单的声明式UI示例（不依赖SDL3高级功能）

#include "declarative_ui.h"
#include "declarative_ui_bindings.h"
#include <stdio.h>

// ============================================================================
// 回调函数
// ============================================================================

static int g_counter = 0;

void on_increment(void* user_data) {
    g_counter++;
    printf("Counter incremented to: %d\n", g_counter);
}

void on_reset(void* user_data) {
    g_counter = 0;
    printf("Counter reset to: %d\n", g_counter);
}

// ============================================================================
// 创建示例UI
// ============================================================================

Widget* create_counter_ui(void) {
    ReactiveState* enabled = create_state_bool(true);
    ReactiveState* count = create_state_float(0.5f);
    
    Widget* children[] = {
        create_text_widget("Simple Counter Demo"),
        create_text_widget(""),
        create_checkbox_widget("Enable Counter", enabled),
        create_slider_widget("Value", count, 0.0f, 1.0f),
        create_text_widget(""),
        create_button_widget("Increment", on_increment),
        create_button_widget("Reset", on_reset),
    };
    
    return create_container_widget(children, sizeof(children) / sizeof(children[0]));
}

// ============================================================================
// 主函数
// ============================================================================

int main(void) {
    printf("Starting Simple Declarative UI Demo...\n");

    // 创建UI
    Widget* root_widget = create_window_widget("Simple Demo", create_counter_ui());
    WidgetTree* tree = create_widget_tree(root_widget);
    
    // 构建树
    printf("Building widget tree...\n");
    build_widget_tree(tree);
    
    printf("Tree built successfully!\n");
    printf("Root widget ID: %u\n", root_widget->widget_id);
    printf("Tree ID: %u\n", tree->tree_id);
    
    // 打印树结构
    printf("\nWidget tree structure:\n");
    print_widget_tree(root_widget, 0);
    
    // 清理
    printf("\nCleaning up...\n");
    release_widget_tree(tree);
    
    printf("Demo finished.\n");
    return 0;
}
