#include <unity.h>
#include "platform/input.h"
#include "core/launcher_return.h"

void test_plain_backspace_stays_back() {
    TEST_ASSERT_EQUAL_INT(INPUT_BACK, mapBackspaceEvent(false, false));
}

void test_fn_backspace_returns_to_launcher_outside_text_input() {
    TEST_ASSERT_EQUAL_INT(INPUT_LAUNCHER, mapBackspaceEvent(true, false));
}

void test_fn_backspace_stays_backspace_during_text_input() {
    TEST_ASSERT_EQUAL_INT(INPUT_BACK, mapBackspaceEvent(true, true));
}

void test_dirty_launcher_request_opens_confirmation() {
    TEST_ASSERT_TRUE(decideLauncherReturn(false, true, true, false, false)
                     == LauncherReturnAction::ShowConfirmation);
}

void test_clean_launcher_request_returns_without_confirmation() {
    TEST_ASSERT_TRUE(decideLauncherReturn(false, false, true, false, false)
                     == LauncherReturnAction::ReturnToLauncher);
}

void test_enter_discards_changes_and_returns() {
    TEST_ASSERT_TRUE(decideLauncherReturn(true, true, false, true, false)
                     == LauncherReturnAction::DiscardAndReturn);
}

void test_escape_cancels_and_stays_in_app() {
    TEST_ASSERT_TRUE(decideLauncherReturn(true, true, false, false, true)
                     == LauncherReturnAction::CancelConfirmation);
}

void test_other_input_keeps_confirmation_open() {
    TEST_ASSERT_TRUE(decideLauncherReturn(true, true, false, false, false)
                     == LauncherReturnAction::Ignore);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_plain_backspace_stays_back);
    RUN_TEST(test_fn_backspace_returns_to_launcher_outside_text_input);
    RUN_TEST(test_fn_backspace_stays_backspace_during_text_input);
    RUN_TEST(test_dirty_launcher_request_opens_confirmation);
    RUN_TEST(test_clean_launcher_request_returns_without_confirmation);
    RUN_TEST(test_enter_discards_changes_and_returns);
    RUN_TEST(test_escape_cancels_and_stays_in_app);
    RUN_TEST(test_other_input_keeps_confirmation_open);
    return UNITY_END();
}
