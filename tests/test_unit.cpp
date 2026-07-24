// ==============================================================================
// windows-media-session-0.8.0 - Unit Tests
// ==============================================================================
// Description: Comprehensive automated test suite
// ==============================================================================

#include <media_session/media_session.h>
#include <media_session/types.h>
#include <media_session/error.h>

#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>

// ==============================================================================
// Test Utilities
// ==============================================================================
int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        std::cout << "[TEST] " #name " ... " << std::flush; \
        try { \
            test_##name(); \
            std::cout << "[PASS]" << std::endl; \
            g_testsPassed++; \
        } catch (const char* msg) { \
            std::cout << "[FAIL] " << msg << std::endl; \
            g_testsFailed++; \
        } catch (...) { \
            std::cout << "[FAIL] Unknown exception" << std::endl; \
            g_testsFailed++; \
        } \
    } \
    void test_##name()

#define ASSERT(condition) \
    if (!(condition)) { \
        throw "Assertion failed: " #condition; \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        throw "Assertion failed: " #a " != " #b; \
    }

#define ASSERT_NE(a, b) \
    if ((a) == (b)) { \
        throw "Assertion failed: " #a " == " #b; \
    }

// ==============================================================================
// Helper: Create Hidden Window for Tests
// ==============================================================================
HWND createTestWindow() {
    static const char* className = "MediaSessionTestWindow";
    static bool registered = false;
    
    if (!registered) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = className;
        RegisterClassA(&wc);
        registered = true;
    }
    
    return CreateWindowExA(
        0, className, "Test", WS_POPUP,
        0, 0, 1, 1, nullptr, nullptr,
        GetModuleHandleA(nullptr), nullptr
    );
}

// ==============================================================================
// Factory Tests
// ==============================================================================
TEST(factory_creates_valid_instance) {
    HWND hwnd = createTestWindow();
    ASSERT(hwnd != nullptr);
    
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    DestroyWindow(hwnd);
}

TEST(factory_handles_invalid_hwnd) {
    // Test with actually invalid HWND (not null, but invalid value)
    // HWND(0x12345678) is not a real window, should be rejected
    HWND fakeHwnd = reinterpret_cast<HWND>(0x12345678);
    auto session = windows_platform::MediaSession::create(fakeHwnd);
    ASSERT(session == nullptr);  // Should reject invalid HWND
}

// ==============================================================================
// Error Handling Tests
// ==============================================================================
TEST(error_info_success_state) {
    auto error = windows_platform::kSuccess;
    ASSERT(error.isSuccess());
    ASSERT(!error.isError());
    ASSERT(error.code == windows_platform::ErrorCode::Success);
}

TEST(error_info_failure_state) {
    windows_platform::ErrorInfo error{
        windows_platform::ErrorCode::InvalidParameter,
        "Test error"
    };
    ASSERT(!error.isSuccess());
    ASSERT(error.isError());
}

// ==============================================================================
// Playback State Tests
// ==============================================================================
TEST(playback_state_default_is_closed) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto state = session->getPlaybackState();
    ASSERT_EQ(state, windows_platform::PlaybackState::Closed);
    
    DestroyWindow(hwnd);
}

TEST(playback_state_can_be_set) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->setPlaybackState(windows_platform::PlaybackState::Playing);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Playing);
    
    error = session->setPlaybackState(windows_platform::PlaybackState::Paused);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Paused);
    
    DestroyWindow(hwnd);
}

TEST(playback_state_all_values) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    // Test all states
    session->setPlaybackState(windows_platform::PlaybackState::Closed);
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Closed);
    
    session->setPlaybackState(windows_platform::PlaybackState::Playing);
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Playing);
    
    session->setPlaybackState(windows_platform::PlaybackState::Paused);
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Paused);
    
    session->setPlaybackState(windows_platform::PlaybackState::Stopped);
    ASSERT_EQ(session->getPlaybackState(), windows_platform::PlaybackState::Stopped);
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Metadata Tests
// ==============================================================================
TEST(metadata_requires_title) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    windows_platform::Metadata meta{};
    meta.title = nullptr; // No title
    
    auto error = session->setMetadata(meta);
    ASSERT(error.isError()); // Should fail
    ASSERT_EQ(error.code, windows_platform::ErrorCode::InvalidParameter);
    
    DestroyWindow(hwnd);
}

TEST(metadata_accepts_valid_data) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    windows_platform::Metadata meta{};
    meta.title = "Test Song";
    meta.artist = "Test Artist";
    meta.album = "Test Album";
    meta.duration = std::chrono::minutes(3) + std::chrono::seconds(30);
    
    auto error = session->setMetadata(meta);
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

TEST(metadata_can_be_cleared) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    windows_platform::Metadata meta{};
    meta.title = "Test Song";
    session->setMetadata(meta);
    
    auto error = session->clearMetadata();
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Thumbnail Tests
// ==============================================================================
TEST(thumbnail_rejects_null_data) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->setThumbnail(nullptr, 100);
    ASSERT(error.isError());
    
    DestroyWindow(hwnd);
}

TEST(thumbnail_rejects_zero_size) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    uint8_t dummy = 0;
    auto error = session->setThumbnail(&dummy, 0);
    ASSERT(error.isError());
    
    DestroyWindow(hwnd);
}

TEST(thumbnail_can_be_cleared) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->clearThumbnail();
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Button Enable Tests
// ==============================================================================
TEST(buttons_can_be_enabled) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->setButtonEnabled(windows_platform::MediaButton::Play, true);
    ASSERT(error.isSuccess());
    
    error = session->setButtonEnabled(windows_platform::MediaButton::Pause, true);
    ASSERT(error.isSuccess());
    
    error = session->setButtonEnabled(windows_platform::MediaButton::Next, true);
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

TEST(buttons_can_be_disabled) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->setButtonEnabled(windows_platform::MediaButton::Previous, false);
    ASSERT(error.isSuccess());
    
    error = session->setButtonEnabled(windows_platform::MediaButton::Stop, false);
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Callback Tests
// ==============================================================================
TEST(callbacks_can_be_registered) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    bool playCalled = false;
    bool pauseCalled = false;
    bool stopCalled = false;
    bool nextCalled = false;
    bool prevCalled = false;
    bool ffCalled = false;
    bool rwCalled = false;
    bool seekCalled = false;
    bool shuffleCalled = false;
    bool repeatCalled = false;
    
    session->onPlay([&] { playCalled = true; });
    session->onPause([&] { pauseCalled = true; });
    session->onStop([&] { stopCalled = true; });
    session->onNext([&] { nextCalled = true; });
    session->onPrevious([&] { prevCalled = true; });
    session->onFastForward([&] { ffCalled = true; });
    session->onRewind([&] { rwCalled = true; });
    session->onSeekRequested([&](std::chrono::milliseconds) { seekCalled = true; });
    session->onShuffleChangeRequested([&](bool) { shuffleCalled = true; });
    session->onRepeatModeChangeRequested([&](windows_platform::RepeatMode) { repeatCalled = true; });
    
    // Note: We can't easily trigger the callbacks in unit tests
    // But we can verify they don't crash
    ASSERT(true);
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Position/Timeline Tests
// ==============================================================================
TEST(position_can_be_set) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    // Use new updateTimeline() method
    auto error = session->updateTimeline(
        std::chrono::seconds(90),
        std::chrono::minutes(5)
    );
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

TEST(seekbar_can_be_toggled) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    auto error = session->enableSeekbar(true);
    ASSERT(error.isSuccess());
    
    error = session->enableSeekbar(false);
    ASSERT(error.isSuccess());
    
    DestroyWindow(hwnd);
}

TEST(shuffle_can_be_toggled) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    // Default should be false
    ASSERT_EQ(session->isShuffleEnabled(), false);
    
    // Enable shuffle
    auto error = session->setShuffleEnabled(true);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->isShuffleEnabled(), true);
    
    // Disable shuffle
    error = session->setShuffleEnabled(false);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->isShuffleEnabled(), false);
    
    DestroyWindow(hwnd);
}

TEST(repeat_mode_can_be_set) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    // Default should be None
    ASSERT_EQ(session->getRepeatMode(), windows_platform::RepeatMode::None);
    
    // Set to Track
    auto error = session->setRepeatMode(windows_platform::RepeatMode::Track);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->getRepeatMode(), windows_platform::RepeatMode::Track);
    
    // Set to List
    error = session->setRepeatMode(windows_platform::RepeatMode::List);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->getRepeatMode(), windows_platform::RepeatMode::List);
    
    // Set back to None
    error = session->setRepeatMode(windows_platform::RepeatMode::None);
    ASSERT(error.isSuccess());
    ASSERT_EQ(session->getRepeatMode(), windows_platform::RepeatMode::None);
    
    DestroyWindow(hwnd);
}

// ==============================================================================
// Integration Tests
// ==============================================================================
TEST(full_integration_workflow) {
    HWND hwnd = createTestWindow();
    auto session = windows_platform::MediaSession::create(hwnd);
    ASSERT(session != nullptr);
    
    // Set metadata
    windows_platform::Metadata meta{};
    meta.title = "Integration Test";
    meta.artist = "Test Artist";
    meta.album = "Test Album";
    auto error = session->setMetadata(meta);
    ASSERT(error.isSuccess());
    
    // Set playback state
    error = session->setPlaybackState(windows_platform::PlaybackState::Playing);
    ASSERT(error.isSuccess());
    
    // Enable buttons
    error = session->setButtonEnabled(windows_platform::MediaButton::Play, true);
    ASSERT(error.isSuccess());
    
    // Register callbacks
    session->onPlay([] {});
    session->onPause([] {});
    
    // Success!
    ASSERT(true);
    
    DestroyWindow(hwnd);
}

TEST(multiple_instances_can_coexist) {
    HWND hwnd1 = createTestWindow();
    HWND hwnd2 = createTestWindow();
    
    auto session1 = windows_platform::MediaSession::create(hwnd1);
    auto session2 = windows_platform::MediaSession::create(hwnd2);
    
    ASSERT(session1 != nullptr);
    ASSERT(session2 != nullptr);
    
    // Both should work independently
    session1->setPlaybackState(windows_platform::PlaybackState::Playing);
    session2->setPlaybackState(windows_platform::PlaybackState::Paused);
    
    ASSERT_EQ(session1->getPlaybackState(), windows_platform::PlaybackState::Playing);
    ASSERT_EQ(session2->getPlaybackState(), windows_platform::PlaybackState::Paused);
    
    DestroyWindow(hwnd1);
    DestroyWindow(hwnd2);
}

// ==============================================================================
// Main Test Runner
// ==============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "windows-media-session-0.8.0" << std::endl;
    std::cout << "Automated Unit Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run all tests
    run_test_factory_creates_valid_instance();
    run_test_factory_handles_invalid_hwnd();
    run_test_error_info_success_state();
    run_test_error_info_failure_state();
    run_test_playback_state_default_is_closed();
    run_test_playback_state_can_be_set();
    run_test_playback_state_all_values();
    run_test_metadata_requires_title();
    run_test_metadata_accepts_valid_data();
    run_test_metadata_can_be_cleared();
    run_test_thumbnail_rejects_null_data();
    run_test_thumbnail_rejects_zero_size();
    run_test_thumbnail_can_be_cleared();
    run_test_buttons_can_be_enabled();
    run_test_buttons_can_be_disabled();
    run_test_callbacks_can_be_registered();
    run_test_position_can_be_set();
    run_test_seekbar_can_be_toggled();
    run_test_shuffle_can_be_toggled();
    run_test_repeat_mode_can_be_set();
    run_test_full_integration_workflow();
    run_test_multiple_instances_can_coexist();
    
    // Summary
    std::cout << "========================================" << std::endl;
    std::cout << "TEST SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[PASS] Passed:  " << g_testsPassed << std::endl;
    std::cout << "[FAIL] Failed:  " << g_testsFailed << std::endl;
    std::cout << "[TOTAL] Total: " << (g_testsPassed + g_testsFailed) << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (g_testsFailed == 0) {
        std::cout << "[SUCCESS] ALL UNIT TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "[FAILED] SOME UNIT TESTS FAILED!" << std::endl;
        return 1;
    }
}
