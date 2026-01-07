// declarative_ui_test.cpp
// 声明式UI框架单元测试

#include "../declarative_ui.h"
#include "../declarative_renderer.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <cmath>

// ============================================================================
// 测试辅助宏
// ============================================================================

#define TEST_START(name) \
    printf("\n=== Test: %s ===\n", name);

#define TEST_PASS() \
    printf("✓ Test passed\n");

#define TEST_FAIL(msg) \
    printf("✗ Test failed: %s\n", msg);

#define ASSERT_TRUE(condition, msg) \
    if (!(condition)) { \
        TEST_FAIL(msg); \
        return false; \
    }

#define ASSERT_FALSE(condition, msg) \
    if (condition) { \
        TEST_FAIL(msg); \
        return false; \
    }

#define ASSERT_EQ(a, b, msg) \
    if ((a) != (b)) { \
        printf("✗ Expected: %d, Got: %d - %s\n", (int)(a), (int)(b), msg); \
        return false; \
    }

#define ASSERT_FLOAT_EQ(a, b, epsilon, msg) \
    if (fabs((a) - (b)) > (epsilon)) { \
        printf("✗ Expected: %.4f, Got: %.4f - %s\n", (double)(a), (double)(b), msg); \
        return false; \
    }

#define ASSERT_STR_EQ(a, b, msg) \
    if (strcmp((a), (b)) != 0) { \
        printf("✗ Expected: '%s', Got: '%s' - %s\n", (a), (b), msg); \
        return false; \
    }

#define ASSERT_NOT_NULL(ptr, msg) \
    if ((ptr) == nullptr) { \
        TEST_FAIL(msg); \
        return false; \
    }

#define ASSERT_NULL(ptr, msg) \
    if ((ptr) != nullptr) { \
        TEST_FAIL(msg); \
        return false; \
    }

// ============================================================================
// 测试计数器
// ============================================================================

static int g_tests_passed = 0;
static int g_tests_failed = 0;

// ============================================================================
// 测试函数声明
// ============================================================================

typedef bool (*TestFn)(void);

// ============================================================================
// 响应式状态系统测试
// ============================================================================

bool test_create_state_bool(void) {
    TEST_START("Create Bool State");
    
    ReactiveState* state = create_state_bool(true);
    ASSERT_NOT_NULL(state, "State should not be null");
    ASSERT_EQ(state->type, STATE_TYPE_BOOL, "State type should be BOOL");
    ASSERT_EQ(state->current.bool_value, true, "Initial value should be true");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_create_state_float(void) {
    TEST_START("Create Float State");
    
    ReactiveState* state = create_state_float(3.14f);
    ASSERT_NOT_NULL(state, "State should not be null");
    ASSERT_EQ(state->type, STATE_TYPE_FLOAT, "State type should be FLOAT");
    ASSERT_FLOAT_EQ(state->current.float_value, 3.14f, 0.001f, "Initial value should be 3.14");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_create_state_int(void) {
    TEST_START("Create Int State");
    
    ReactiveState* state = create_state_int(42);
    ASSERT_NOT_NULL(state, "State should not be null");
    ASSERT_EQ(state->type, STATE_TYPE_INT, "State type should be INT");
    ASSERT_EQ(state->current.int_value, 42, "Initial value should be 42");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_create_state_string(void) {
    TEST_START("Create String State");
    
    ReactiveState* state = create_state_string("Hello");
    ASSERT_NOT_NULL(state, "State should not be null");
    ASSERT_EQ(state->type, STATE_TYPE_STRING, "State type should be STRING");
    ASSERT_STR_EQ(state->current.string_value, "Hello", "Initial value should be 'Hello'");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_set_state_bool(void) {
    TEST_START("Set Bool State");
    
    ReactiveState* state = create_state_bool(false);
    ASSERT_EQ(state->current.bool_value, false, "Initial value should be false");
    
    set_state_bool(state, true);
    ASSERT_EQ(state->current.bool_value, true, "Value should be true after set");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_set_state_float(void) {
    TEST_START("Set Float State");
    
    ReactiveState* state = create_state_float(1.0f);
    ASSERT_FLOAT_EQ(state->current.float_value, 1.0f, 0.001f, "Initial value should be 1.0");
    
    set_state_float(state, 2.5f);
    ASSERT_FLOAT_EQ(state->current.float_value, 2.5f, 0.001f, "Value should be 2.5 after set");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_state_listener(void) {
    TEST_START("State Listener");
    
    ReactiveState* state = create_state_bool(false);
    int callback_count = 0;
    
    auto listener = [](ReactiveState* state, void* user_data) {
        int* count = (int*)user_data;
        (*count)++;
    };
    
    add_state_listener(state, listener, &callback_count);
    ASSERT_EQ(callback_count, 0, "Callback count should be 0 before state change");
    
    set_state_bool(state, true);
    ASSERT_EQ(callback_count, 1, "Callback count should be 1 after state change");
    
    remove_state_listener(state, listener);
    set_state_bool(state, false);
    ASSERT_EQ(callback_count, 1, "Callback count should still be 1 after listener removal");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

// ============================================================================
// Widget系统测试
// ============================================================================

bool test_create_text_widget(void) {
    TEST_START("Create Text Widget");
    
    Widget* widget = create_text_widget("Hello World");
    ASSERT_NOT_NULL(widget, "Widget should not be null");
    ASSERT_EQ(widget->type, WIDGET_TYPE_TEXT, "Widget type should be TEXT");
    
    Property* text_prop = widget_get_property(widget, "text");
    ASSERT_NOT_NULL(text_prop, "Text property should not be null");
    ASSERT_STR_EQ(get_property_string(text_prop), "Hello World", "Text should be 'Hello World'");
    
    release_widget(widget);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_create_button_widget(void) {
    TEST_START("Create Button Widget");
    
    Widget* widget = create_button_widget("Click Me", nullptr);
    ASSERT_NOT_NULL(widget, "Widget should not be null");
    ASSERT_EQ(widget->type, WIDGET_TYPE_BUTTON, "Widget type should be BUTTON");
    
    Property* label_prop = widget_get_property(widget, "label");
    ASSERT_NOT_NULL(label_prop, "Label property should not be null");
    ASSERT_STR_EQ(get_property_string(label_prop), "Click Me", "Label should be 'Click Me'");
    
    release_widget(widget);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_create_checkbox_widget(void) {
    TEST_START("Create Checkbox Widget");
    
    ReactiveState* state = create_state_bool(true);
    Widget* widget = create_checkbox_widget("Check Me", state);
    ASSERT_NOT_NULL(widget, "Widget should not be null");
    ASSERT_EQ(widget->type, WIDGET_TYPE_CHECKBOX, "Widget type should be CHECKBOX");
    
    Property* label_prop = widget_get_property(widget, "label");
    ASSERT_NOT_NULL(label_prop, "Label property should not be null");
    ASSERT_STR_EQ(get_property_string(label_prop), "Check Me", "Label should be 'Check Me'");
    
    release_widget(widget);
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_widget_tree_build(void) {
    TEST_START("Widget Tree Build");
    
    Widget* root = create_window_widget("My Window", 
        create_container_widget((Widget*[]){
            create_text_widget("Hello"),
            create_button_widget("Click", nullptr)
        }, 2)
    );
    
    WidgetTree* tree = create_widget_tree(root);
    build_widget_tree(tree);
    
    ASSERT_NOT_NULL(tree->root_element, "Root element should exist");
    ASSERT_TRUE(tree->root_element->is_mounted, "Root element should be mounted");
    
    release_widget_tree(tree);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

bool test_reactive_state_update(void) {
    TEST_START("Reactive State Update");
    
    ReactiveState* state = create_state_bool(false);
    bool updated = false;
    
    auto listener = [](ReactiveState* state, void* user_data) {
        bool* updated = (bool*)user_data;
        *updated = true;
    };
    
    add_state_listener(state, listener, &updated);
    set_state_bool(state, true);
    
    ASSERT_TRUE(updated, "State should trigger listener update");
    
    release_state(state);
    TEST_PASS();
    g_tests_passed++;
    return true;
}

// ============================================================================
// 测试运行器
// ============================================================================

void run_all_tests(void) {
    TestFn tests[] = {
        // 响应式状态系统测试
        test_create_state_bool,
        test_create_state_float,
        test_create_state_int,
        test_create_state_string,
        test_set_state_bool,
        test_set_state_float,
        test_state_listener,
        
        // Widget系统测试
        test_create_text_widget,
        test_create_button_widget,
        test_create_checkbox_widget,
        test_widget_tree_build,
        test_reactive_state_update,
    };
    
    const int test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("\n");
    printf("========================================\n");
    printf("  Declarative UI Framework Tests\n");
    printf("========================================\n");
    printf("Running %d tests...\n\n", test_count);
    
    g_tests_passed = 0;
    g_tests_failed = 0;
    
    for (int i = 0; i < test_count; i++) {
        try {
            if (!tests[i]()) {
                g_tests_failed++;
            }
        } catch (...) {
            printf("✗ Test %d threw an exception\n", i + 1);
            g_tests_failed++;
        }
    }
    
    printf("\n");
    printf("========================================\n");
    printf("  Test Results\n");
    printf("========================================\n");
    printf("Passed: %d\n", g_tests_passed);
    printf("Failed: %d\n", g_tests_failed);
    printf("Total:  %d\n", test_count);
    printf("========================================\n");
    
    if (g_tests_failed == 0) {
        printf("\n✓ All tests passed!\n\n");
    } else {
        printf("\n✗ Some tests failed\n\n");
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main(void) {
    run_all_tests();
    
    return (g_tests_failed == 0) ? 0 : 1;
}
