#include <cassert>
#include <iostream>

#include "oikumene/app/input_policy.hpp"

namespace {

void TestLayerHotkeysIgnoreSystemModifiers() {
    using namespace oikumene;

    assert(LayerHotkeysAllowed({}));
    assert(!LayerHotkeysAllowed({.command = true}));
    assert(!LayerHotkeysAllowed({.control = true}));
    assert(!LayerHotkeysAllowed({.alt = true}));
    assert(!LayerHotkeysAllowed({.shift = true}));
}

void TestAppHotkeysIgnoreSystemModifiersButAllowShift() {
    using namespace oikumene;

    assert(AppHotkeysAllowed({}));
    assert(AppHotkeysAllowed({.shift = true}));
    assert(!AppHotkeysAllowed({.command = true}));
    assert(!AppHotkeysAllowed({.control = true}));
    assert(!AppHotkeysAllowed({.alt = true}));
}

void TestShiftNStillUsesLargeStep() {
    using namespace oikumene;

    assert(!UsesLargeStep({}));
    assert(UsesLargeStep({.shift = true}));
    assert(!UsesLargeStep({.command = true, .shift = true}));
}

void TestScreenshotPausesAutoRunWhenConfigured() {
    using namespace oikumene;

    assert(ShouldPauseForScreenshot(true));
    assert(!ShouldPauseForScreenshot(false));
}

void TestFocusLossPausesAutoRunWhenConfigured() {
    using namespace oikumene;

    assert(ShouldPauseForFocusLoss(true, true, false));
    assert(!ShouldPauseForFocusLoss(false, true, false));
    assert(!ShouldPauseForFocusLoss(true, false, false));
    assert(!ShouldPauseForFocusLoss(true, true, true));
}

} // namespace

int main() {
    TestLayerHotkeysIgnoreSystemModifiers();
    TestAppHotkeysIgnoreSystemModifiersButAllowShift();
    TestShiftNStillUsesLargeStep();
    TestScreenshotPausesAutoRunWhenConfigured();
    TestFocusLossPausesAutoRunWhenConfigured();

    std::cout << "oikumene_input_policy_tests passed\n";
    return 0;
}
