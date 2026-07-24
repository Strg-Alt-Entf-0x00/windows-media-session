// ==============================================================================
// windows-media-session-0.8.0 - Basic Player Example
// ==============================================================================
// Description: Simple example demonstrating MediaSession API
// License:     MIT
// ==============================================================================

#include <media_session/media_session.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>  // For HWND creation

// Simple hidden window for SystemMediaTransportControls
HWND createHiddenWindow() {
    const char* className = "MediaSessionHiddenWindow";
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = className;
    
    RegisterClassA(&wc);
    
    // Create a TOP-LEVEL window (not HWND_MESSAGE)
    // IMPORTANT: SystemMediaTransportControls requires a top-level window,
    // not a message-only window. The window can be hidden (WS_POPUP, no WS_VISIBLE).
    HWND hwnd = CreateWindowExA(
        0,                    // Extended style
        className,            // Class name
        "MediaSession",       // Window title
        WS_POPUP,             // Style: Popup window (no taskbar, no frame)
        0, 0, 1, 1,           // Position and size (minimal, hidden)
        nullptr,              // No parent
        nullptr,              // No menu
        GetModuleHandleA(nullptr),
        nullptr
    );
    
    return hwnd;
}

int main() {
    std::cout << "windows-media-session-0.8.0 - Basic Player Example\n";
    std::cout << "==================================================\n\n";
    
    // Create hidden window for SystemMediaTransportControls
    std::cout << "[0] Creating hidden window (required for desktop apps)...\n";
    HWND hwnd = createHiddenWindow();
    if (!hwnd) {
        std::cerr << "[ERROR] Failed to create window\n";
        return 1;
    }
    std::cout << "[OK] Hidden window created (HWND: " << hwnd << ")\n\n";
    
    // Create MediaSession
    std::cout << "[1] Creating MediaSession...\n";
    auto session = windows_platform::MediaSession::create(hwnd);
    
    if (!session) {
        std::cerr << "[ERROR] Failed to create MediaSession\n";
        std::cerr << "        Requires Windows 10 Build 14393+\n";
        DestroyWindow(hwnd);
        return 1;
    }
    
    std::cout << "[OK] MediaSession created successfully\n\n";
    
    // Set metadata
    std::cout << "[2] Setting metadata...\n";
    windows_platform::Metadata meta;
    meta.title = "Test Song";
    meta.artist = "Test Artist";
    meta.album = "Test Album";
    meta.duration = std::chrono::minutes(3) + std::chrono::seconds(30);
    
    auto error = session->setMetadata(meta);
    if (!error.isSuccess()) {
        std::cerr << "[ERROR] setMetadata failed: " << error.message << "\n";
        DestroyWindow(hwnd);
        return 1;
    }
    
    std::cout << "[OK] Metadata set: \"" << meta.title << "\" by " << meta.artist << "\n\n";
    
    // Register callbacks
    std::cout << "[3] Registering event callbacks...\n";
    session->onPlay([] {
        std::cout << "[EVENT] Play button pressed\n";
    });
    session->onPause([] {
        std::cout << "[EVENT] Pause button pressed\n";
    });
    session->onStop([] {
        std::cout << "[EVENT] Stop button pressed\n";
    });
    session->onNext([] {
        std::cout << "[EVENT] Next button pressed\n";
    });
    session->onPrevious([] {
        std::cout << "[EVENT] Previous button pressed\n";
    });
    
    std::cout << "[OK] Callbacks registered\n\n";
    
    // Set playback state
    std::cout << "[4] Setting playback state to Playing...\n";
    error = session->setPlaybackState(windows_platform::PlaybackState::Playing);
    if (!error.isSuccess()) {
        std::cerr << "[ERROR] setPlaybackState failed: " << error.message << "\n";
        DestroyWindow(hwnd);
        return 1;
    }
    
    std::cout << "[OK] Playback state set to Playing\n\n";
    
    // Keep alive (press Ctrl+C to exit)
    std::cout << "========================================\n";
    std::cout << "Media controls are now active!\n";
    std::cout << "Open Windows Quick Settings (Win+A) or Lock Screen to see controls\n";
    std::cout << "Press Ctrl+C to exit\n";
    std::cout << "========================================\n\n";
    
    // Keep application alive for 5 minutes (or until Ctrl+C)
    // Process Windows messages to receive callbacks
    MSG msg;
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::minutes(5)) {
        // Process Windows messages (required for callbacks)
        while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n[EXIT] Shutting down MediaSession...\n";
    session.reset();  // Destroy session before window
    DestroyWindow(hwnd);
    return 0;
}
