#pragma once

namespace oikumene {

struct ShortcutModifiers {
    bool command = false;
    bool control = false;
    bool alt = false;
    bool shift = false;
};

[[nodiscard]] bool LayerHotkeysAllowed(const ShortcutModifiers& modifiers);
[[nodiscard]] bool AppHotkeysAllowed(const ShortcutModifiers& modifiers);
[[nodiscard]] bool UsesLargeStep(const ShortcutModifiers& modifiers);
[[nodiscard]] bool ShouldPauseForScreenshot(bool pause_on_screenshot);
[[nodiscard]] bool ShouldPauseForFocusLoss(bool pause_on_focus_loss, bool was_focused, bool is_focused);

} // namespace oikumene
