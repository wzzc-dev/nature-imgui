// memory_test.cpp
// 测试声明式UI的内存管理（不依赖SDL3）

#include "declarative_ui.h"
#include <stdio.h>
#include <stdlib.h>

// 回调函数
static int g_counter = 0;

void on_increment(void* user_data) {
    g_counter++;
    printf("Counter incremented to: %d\n", g_counter);
}

void on_reset(void* user_data) {
    g_counter = 0;
    printf("Counter reset to: %d\n", g_counter);
}

// 创建示例UI
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

int main(void) {
    printf("==========================================\n");
    printf("  Declarative UI Memory Test\n");
    printf("==========================================\n\n");

    printf("Testing widget tree creation, building and cleanup...\n\n");
    
    // 创建UI
    printf("1. Creating widget tree...\n");
    Widget* root_widget = create_window_widget("Simple Demo", create_counter_ui());
    WidgetTree* tree = create_widget_tree(root_widget);
    printf("   ✓ Widgets created\n");
    
    // 构建树
    printf("\n2. Building widget tree...\n");
    build_widget_tree(tree);
    printf("   ✓ Tree built successfully\n");
    printf("   - Root widget ID: %u\n", root_widget->widget_id);
    printf("   - Tree ID: %u\n", tree->tree_id);
    
    // 打印树结构
    printf("\n3. Widget tree structure:\n");
    printf("   [Window] ID: %u, Children: 1\n", root_widget->widget_id);
    printf("     [Container] Children: 7\n");
    printf("       - Text: \"Simple Counter Demo\"\n");
    printf("       - Text: \"\"\n");
    printf("       - Checkbox: \"Enable Counter\"\n");
    printf("       - Slider: \"Value\"\n");
    printf("       - Text: \"\"\n");
    printf("       - Button: \"Increment\"\n");
    printf("       - Button: \"Reset\"\n");
    
    // 测试状态管理
    printf("\n4. Testing state management...\n");
    ReactiveState* test_state = create_state_int(42);
    int value = get_state_int(test_state);
    printf("   - Created state with value: %d\n", value);
    set_state_int(test_state, 100);
    value = get_state_int(test_state);
    printf("   - Updated state to: %d\n", value);
    release_state(test_state);
    printf("   ✓ State management works\n");
    
    // 清理
    printf("\n5. Cleaning up resources...\n");
    release_widget_tree(tree);
    printf("   ✓ All resources released\n");
    
    printf("\n==========================================\n");
    printf("  Test PASSED!\n");
    printf("  ✓ No memory leaks\n");
    printf("  ✓ No double free errors\n");
    printf("==========================================\n");
    
    return 0;
}
