// simple_test.cpp
// 简单的声明式UI测试
#include "../declarative_ui.h"
#include <stdio.h>

int main(void) {
    printf("=== Simple Declarative UI Test ===\n\n");
    
    // 测试1: 创建状态
    printf("Test 1: Create state\n");
    ReactiveState* state = create_state_bool(true);
    if (state == nullptr) {
        printf("✗ Failed: State is null\n");
        return 1;
    }
    printf("✓ State created successfully\n");
    printf("  State ID: %u\n", state->state_id);
    printf("  State type: %d (BOOL)\n", state->type);
    printf("  State value: %s\n", state->current.bool_value ? "true" : "false");
    
    // 测试2: 读取状态
    printf("\nTest 2: Read state\n");
    bool value = get_state_bool(state);
    if (value != true) {
        printf("✗ Failed: Expected true, got %s\n", value ? "true" : "false");
        return 1;
    }
    printf("✓ State value read correctly: %s\n", value ? "true" : "false");
    
    // 测试3: 更新状态
    printf("\nTest 3: Update state\n");
    set_state_bool(state, false);
    value = get_state_bool(state);
    if (value != false) {
        printf("✗ Failed: Expected false, got %s\n", value ? "true" : "false");
        return 1;
    }
    printf("✓ State updated successfully: %s\n", value ? "true" : "false");
    
    // 测试4: 创建Widget
    printf("\nTest 4: Create widget\n");
    Widget* text_widget = create_text_widget("Hello World");
    if (text_widget == nullptr) {
        printf("✗ Failed: Widget is null\n");
        return 1;
    }
    printf("✓ Widget created successfully\n");
    printf("  Widget ID: %u\n", text_widget->widget_id);
    printf("  Widget type: %d (TEXT)\n", text_widget->type);
    
    // 测试5: 创建WidgetTree
    printf("\nTest 5: Create widget tree\n");
    WidgetTree* tree = create_widget_tree(text_widget);
    if (tree == nullptr) {
        printf("✗ Failed: Widget tree is null\n");
        return 1;
    }
    printf("✓ Widget tree created successfully\n");
    printf("  Tree ID: %u\n", tree->tree_id);
    
    // 测试6: 构建WidgetTree
    printf("\nTest 6: Build widget tree\n");
    build_widget_tree(tree);
    if (tree->root_element == nullptr) {
        printf("✗ Failed: Root element is null\n");
        return 1;
    }
    printf("✓ Widget tree built successfully\n");
    printf("  Root element ID: %u\n", tree->root_element->element_id);
    printf("  Root element mounted: %s\n", tree->root_element->is_mounted ? "true" : "false");
    
    // 测试7: 释放资源
    printf("\nTest 7: Release resources\n");
    release_widget_tree(tree);
    release_state(state);
    printf("✓ Resources released successfully\n");
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}
