// ==============================================================================
// windows-media-session-0.8.0 - COMPLETE 100% FEATURE DEMO
// ==============================================================================
// Description: Demonstrates ALL features including new shuffle/repeat/timeline
// License:     MIT
// ==============================================================================

#include <media_session/media_session.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <windows.h>

using namespace windows_platform;

// Create hidden window
HWND createHiddenWindow() {
    const char* className = "MediaSessionCompleteDemo";
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = className;
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowExA(
        0, className, "Complete Demo",
        WS_POPUP, 0, 0, 1, 1,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr
    );
    
    return hwnd;
}

// Generate minimal demo PNG
std::vector<uint8_t> generateDemoPNG() {
    return {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xDE,
        0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41, 0x54,
        0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00, 0x00,
        0x03, 0x01, 0x01, 0x00, 0x18, 0xDD, 0x8D, 0xB4,
        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44,
        0xAE, 0x42, 0x60, 0x82
    };
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Windows Media Session - 100% COMPLETE DEMO! 🌟        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    // Create window
    std::cout << "🔧 Creating window...\n";
    HWND hwnd = createHiddenWindow();
    if (!hwnd) {
        std::cerr << "❌ Failed to create window!\n";
        return 1;
    }
    std::cout << "✅ Window created\n\n";
    
    // Create MediaSession
    std::cout << "🎵 Creating MediaSession...\n";
    auto session = MediaSession::create(hwnd);
    if (!session) {
        std::cerr << "❌ Failed to create MediaSession!\n";
        DestroyWindow(hwnd);
        return 1;
    }
    std::cout << "✅ MediaSession created\n\n";
    
    // =========================================================================
    // FEATURE 1: Enhanced Metadata (with track number)
    // =========================================================================
    std::cout << "📝 [1/8] Setting Enhanced Metadata...\n";
    Metadata meta;
    meta.title = "Bohemian Rhapsody";
    meta.artist = "Queen";
    meta.album = "A Night at the Opera";
    meta.trackNumber = 11;  // ✨ NEW!
    meta.appMediaId = "queen-bohemian-rhapsody-1975";  // ✨ NEW!
    
    session->setMetadata(meta);
    std::cout << "   ✅ Title: " << meta.title << "\n";
    std::cout << "   ✅ Artist: " << meta.artist << "\n";
    std::cout << "   ✅ Album: " << meta.album << "\n";
    std::cout << "   ✨ Track #: " << meta.trackNumber << " (NEW!)\n";
    std::cout << "   ✨ Media ID: " << meta.appMediaId << " (NEW!)\n\n";
    
    // =========================================================================
    // FEATURE 2: Album Art
    // =========================================================================
    std::cout << "🖼️  [2/8] Setting Album Art...\n";
    auto thumbnailData = generateDemoPNG();
    session->setThumbnail(thumbnailData.data(), thumbnailData.size());
    std::cout << "   ✅ Thumbnail set (" << thumbnailData.size() << " bytes)\n\n";
    
    // =========================================================================
    // FEATURE 3: Shuffle Control
    // =========================================================================
    std::cout << "🔀 [3/8] Setting Shuffle Mode...\n";
    session->setShuffleEnabled(true);  // ✨ NEW!
    std::cout << "   ✨ Shuffle: " << (session->isShuffleEnabled() ? "ON" : "OFF") << " (NEW!)\n\n";
    
    // =========================================================================
    // FEATURE 4: Repeat Mode
    // =========================================================================
    std::cout << "🔁 [4/8] Setting Repeat Mode...\n";
    session->setRepeatMode(RepeatMode::List);  // ✨ NEW!
    std::cout << "   ✨ Repeat: ";
    switch (session->getRepeatMode()) {
        case RepeatMode::None: std::cout << "OFF"; break;
        case RepeatMode::Track: std::cout << "ONE"; break;
        case RepeatMode::List: std::cout << "ALL"; break;
    }
    std::cout << " (NEW!)\n\n";
    
    // =========================================================================
    // FEATURE 5: Timeline with Position
    // =========================================================================
    std::cout << "⏱️  [5/8] Setting Timeline...\n";
    auto duration = std::chrono::minutes(5) + std::chrono::seconds(55);
    auto position = std::chrono::seconds(90);
    session->updateTimeline(position, duration);  // ✨ NEW!
    std::cout << "   ✨ Duration: 5:55 (NEW updateTimeline method!)\n";
    std::cout << "   ✨ Position: 1:30 (NEW!)\n\n";
    
    // =========================================================================
    // FEATURE 6: All 10 Callbacks
    // =========================================================================
    std::cout << "🎮 [6/8] Registering ALL 10 Callbacks...\n";
    
    session->onPlay([] {
        std::cout << "   ▶️  CALLBACK: Play\n";
    });
    session->onPause([] {
        std::cout << "   ⏸️  CALLBACK: Pause\n";
    });
    session->onStop([] {
        std::cout << "   ⏹️  CALLBACK: Stop\n";
    });
    session->onNext([] {
        std::cout << "   ⏭️  CALLBACK: Next\n";
    });
    session->onPrevious([] {
        std::cout << "   ⏮️  CALLBACK: Previous\n";
    });
    session->onFastForward([] {  // ✨ NEW!
        std::cout << "   ⏩ CALLBACK: FastForward (NEW!)\n";
    });
    session->onRewind([] {  // ✨ NEW!
        std::cout << "   ⏪ CALLBACK: Rewind (NEW!)\n";
    });
    session->onSeekRequested([](std::chrono::milliseconds pos) {  // ✨ NEW!
        int sec = static_cast<int>(pos.count() / 1000);
        std::cout << "   🎯 CALLBACK: Seek to " << (sec/60) << ":" 
                  << (sec%60 < 10 ? "0" : "") << (sec%60) << " (NEW!)\n";
    });
    session->onShuffleChangeRequested([&session](bool enabled) {  // ✨ NEW!
        session->setShuffleEnabled(enabled);
        std::cout << "   🔀 CALLBACK: Shuffle " << (enabled ? "ON" : "OFF") << " (NEW!)\n";
    });
    session->onRepeatModeChangeRequested([&session](RepeatMode mode) {  // ✨ NEW!
        session->setRepeatMode(mode);
        std::cout << "   🔁 CALLBACK: Repeat mode changed (NEW!)\n";
    });
    
    std::cout << "   ✅ 5 Basic callbacks\n";
    std::cout << "   ✨ 5 NEW callbacks (FF, RW, Seek, Shuffle, Repeat)\n\n";
    
    // =========================================================================
    // FEATURE 7: Playback State
    // =========================================================================
    std::cout << "▶️  [7/8] Starting Playback...\n";
    session->setPlaybackState(PlaybackState::Playing);
    std::cout << "   ✅ State: PLAYING\n\n";
    
    // =========================================================================
    // FEATURE 8: Enable All Buttons
    // =========================================================================
    std::cout << "🎛️  [8/8] Enabling All Media Buttons...\n";
    session->setButtonEnabled(MediaButton::Play, true);
    session->setButtonEnabled(MediaButton::Pause, true);
    session->setButtonEnabled(MediaButton::Stop, true);
    session->setButtonEnabled(MediaButton::Next, true);
    session->setButtonEnabled(MediaButton::Previous, true);
    session->setButtonEnabled(MediaButton::FastForward, true);
    session->setButtonEnabled(MediaButton::Rewind, true);
    std::cout << "   ✅ All 7 buttons enabled\n\n";
    
    // =========================================================================
    // Instructions
    // =========================================================================
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🎵 ALL FEATURES ACTIVE - TEST THEM NOW! 🎵            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "📱 HOW TO TEST:\n\n";
    
    std::cout << "1️⃣  QUICK SETTINGS (Easiest!):\n";
    std::cout << "   • Press Win+A\n";
    std::cout << "   • See media controls in bottom-right\n";
    std::cout << "   • Try: Play, Pause, Next, Previous\n";
    std::cout << "   • Try: 🔀 Shuffle toggle (NEW!)\n";
    std::cout << "   • Try: 🔁 Repeat toggle (NEW!)\n";
    std::cout << "   • Try: Seek bar drag (NEW!)\n\n";
    
    std::cout << "2️⃣  LOCK SCREEN:\n";
    std::cout << "   • Press Win+L to lock\n";
    std::cout << "   • See full controls with album art\n";
    std::cout << "   • Timeline shows 1:30 / 5:55\n\n";
    
    std::cout << "3️⃣  HARDWARE KEYS:\n";
    std::cout << "   • Use keyboard media keys\n";
    std::cout << "   • See callbacks in console!\n\n";
    
    std::cout << "4️⃣  VOLUME OVERLAY (Windows 11):\n";
    std::cout << "   • Press any media key\n";
    std::cout << "   • See popup with song info\n\n";
    
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ⏱️  Running for 3 minutes - Watch console for events!  ║\n";
    std::cout << "║  Press Ctrl+C to exit anytime                           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    // =========================================================================
    // Keep alive and update position
    // =========================================================================
    auto startTime = std::chrono::steady_clock::now();
    int lastSecond = -1;
    
    while (std::chrono::steady_clock::now() - startTime < std::chrono::minutes(3)) {
        // Process Windows messages (for callbacks)
        MSG msg;
        while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        // Update timeline position every 5 seconds
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        int currentSecond = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(elapsed).count()
        );
        
        if (currentSecond != lastSecond && currentSecond % 5 == 0) {
            auto newPosition = std::chrono::seconds(90 + currentSecond);
            session->updateTimeline(newPosition, duration);
            
            int min = (90 + currentSecond) / 60;
            int sec = (90 + currentSecond) % 60;
            std::cout << "⏱️  Timeline updated: " << min << ":" 
                      << (sec < 10 ? "0" : "") << sec << " / 5:55\n";
            lastSecond = currentSecond;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✅ Demo completed successfully!                        ║\n";
    std::cout << "║  🌟 ALL 100% FEATURES DEMONSTRATED!                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    session.reset();
    DestroyWindow(hwnd);
    return 0;
}
