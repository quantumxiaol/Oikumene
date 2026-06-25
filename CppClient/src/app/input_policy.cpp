#include "oikumene/app/input_policy.hpp"

namespace oikumene {

bool LayerHotkeysAllowed(const ShortcutModifiers& modifiers) {
    return !modifiers.command && !modifiers.control && !modifiers.alt && !modifiers.shift;
}

bool AppHotkeysAllowed(const ShortcutModifiers& modifiers) {
    return !modifiers.command && !modifiers.control && !modifiers.alt;
}

bool UsesLargeStep(const ShortcutModifiers& modifiers) {
    return AppHotkeysAllowed(modifiers) && modifiers.shift;
}

bool ShouldPauseForScreenshot(bool pause_on_screenshot) {
    return pause_on_screenshot;
}

bool ShouldPauseForFocusLoss(bool pause_on_focus_loss, bool was_focused, bool is_focused) {
    return pause_on_focus_loss && was_focused && !is_focused;
}

} // namespace oikumene
