#pragma once

enum class LauncherReturnAction {
    Ignore,
    ShowConfirmation,
    ReturnToLauncher,
    DiscardAndReturn,
    CancelConfirmation,
};

constexpr LauncherReturnAction decideLauncherReturn(
    bool confirmationOpen,
    bool projectDirty,
    bool launcherRequested,
    bool confirmPressed,
    bool cancelPressed) {
    if (confirmationOpen) {
        if (confirmPressed) return LauncherReturnAction::DiscardAndReturn;
        if (cancelPressed) return LauncherReturnAction::CancelConfirmation;
        return LauncherReturnAction::Ignore;
    }

    if (!launcherRequested) return LauncherReturnAction::Ignore;
    return projectDirty
        ? LauncherReturnAction::ShowConfirmation
        : LauncherReturnAction::ReturnToLauncher;
}
