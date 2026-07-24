// ==============================================================================
// windows-media-session-0.8.0 - Advanced Player Example with Album Art
// ==============================================================================
// Description: Full-featured example with thumbnail support
// License:     MIT
// ==============================================================================

#include <media_session/media_session.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <windows.h>

// Create hidden window
HWND createHiddenWindow() {
    const char* className = "MediaSessionAdvancedWindow";
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = className;
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowExA(
        0,
        className,
        "MediaSession Advanced",
        WS_POPUP,
        0, 0, 1, 1,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );
    
    return hwnd;
}

// Load image file into memory
std::vector<uint8_t> loadImageFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return {};
    }
    
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return data;
}

// Generate a simple 300x300 PNG in memory (solid color square)
// This is a minimal valid PNG for demonstration
std::vector<uint8_t> generateDemoPNG() {
    // Minimal 1x1 PNG (red pixel) - valid PNG structure
    std::vector<uint8_t> png = {
        // PNG signature
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
        // IHDR chunk (13 bytes)
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,  // 1x1 dimensions
        0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xDE,
        // IDAT chunk (12 bytes)
        0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41, 0x54,
        0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00, 0x00,  // Red pixel data
        0x03, 0x01, 0x01, 0x00, 0x18, 0xDD, 0x8D, 0xB4,
        // IEND chunk
        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44,
        0xAE, 0x42, 0x60, 0x82
    };
    
    return png;
}

int main() {
    std::cout << "windows-media-session-0.8.0 - Advanced Player Example\n";
    std::cout << "======================================================\n\n";
    
    // Create hidden window
    std::cout << "[0] Creating hidden window...\n";
    HWND hwnd = createHiddenWindow();
    if (!hwnd) {
        std::cerr << "[ERROR] Failed to create window\n";
        return 1;
    }
    std::cout << "[OK] Window created\n\n";
    
    // Create MediaSession
    std::cout << "[1] Creating MediaSession...\n";
    auto session = windows_platform::MediaSession::create(hwnd);
    
    if (!session) {
        std::cerr << "[ERROR] Failed to create MediaSession\n";
        DestroyWindow(hwnd);
        return 1;
    }
    std::cout << "[OK] MediaSession created\n\n";
    
    // Set metadata
    std::cout << "[2] Setting metadata...\n";
    windows_platform::Metadata meta;
    meta.title = "Bohemian Rhapsody";
    meta.artist = "Queen";
    meta.album = "A Night at the Opera";
    meta.duration = std::chrono::minutes(5) + std::chrono::seconds(55);
    
    auto error = session->setMetadata(meta);
    if (!error.isSuccess()) {
        std::cerr << "[ERROR] setMetadata failed: " << error.message << "\n";
        DestroyWindow(hwnd);
        return 1;
    }
    std::cout << "[OK] Metadata set\n\n";
    
    // Set thumbnail (try to load from file, or generate demo)
    std::cout << "[3] Setting album art...\n";
    
    // Try to load cover.png if it exists
    auto thumbnailData = loadImageFile("cover.png");
    
    if (thumbnailData.empty()) {
        std::cout << "    No cover.png found, generating demo thumbnail...\n";
        thumbnailData = generateDemoPNG();
    }
    
    if (!thumbnailData.empty()) {
        error = session->setThumbnail(thumbnailData.data(), thumbnailData.size());
        if (!error.isSuccess()) {
            std::cerr << "[WARNING] setThumbnail failed: " << error.message << "\n";
        } else {
            std::cout << "[OK] Album art set (" << thumbnailData.size() << " bytes)\n";
        }
    }
    std::cout << "\n";
    
    // Register callbacks
    std::cout << "[4] Registering callbacks...\n";
    session->onPlay([] {
        std::cout << "[EVENT] ▶️  Play pressed\n";
    });
    session->onPause([] {
        std::cout << "[EVENT] ⏸️  Pause pressed\n";
    });
    session->onStop([] {
        std::cout << "[EVENT] ⏹️  Stop pressed\n";
    });
    session->onNext([] {
        std::cout << "[EVENT] ⏭️  Next pressed\n";
    });
    session->onPrevious([] {
        std::cout << "[EVENT] ⏮️  Previous pressed\n";
    });
    std::cout << "[OK] Callbacks registered\n\n";
    
    // Set playback state
    std::cout << "[5] Starting playback...\n";
    error = session->setPlaybackState(windows_platform::PlaybackState::Playing);
    if (!error.isSuccess()) {
        std::cerr << "[ERROR] setPlaybackState failed: " << error.message << "\n";
        DestroyWindow(hwnd);
        return 1;
    }
    std::cout << "[OK] Playback started\n\n";
    
    // Instructions
    std::cout << "========================================\n";
    std::cout << "🎵 Advanced Media Controls Demo\n";
    std::cout << "========================================\n";
    std::cout << "Now playing: " << meta.title << " by " << meta.artist << "\n";
    std::cout << "Album: " << meta.album << "\n\n";
    std::cout << "✨ Features Active:\n";
    std::cout << "  ✅ Metadata display\n";
    std::cout << "  ✅ Album art thumbnail\n";
    std::cout << "  ✅ Playback state\n";
    std::cout << "  ✅ Media button events\n\n";
    std::cout << "📱 Test it:\n";
    std::cout << "  • Press Win+A for Quick Settings\n";
    std::cout << "  • Lock screen (Win+L) to see on Lock Screen\n";
    std::cout << "  • Use hardware media keys\n";
    std::cout << "  • Connect Bluetooth headphones/car\n\n";
    std::cout << "Press Ctrl+C to exit\n";
    std::cout << "========================================\n\n";
    
    // Simulate playback with position updates
    std::cout << "Simulating playback...\n\n";
    
    auto startTime = std::chrono::steady_clock::now();
    int lastSecond = -1;
    
    while (std::chrono::steady_clock::now() - startTime < std::chrono::minutes(2)) {
        // Process Windows messages
        MSG msg;
        while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        // Update position every second
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        int currentSecond = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
        
        if (currentSecond != lastSecond && currentSecond % 10 == 0) {
            // Print status every 10 seconds
            int minutes = currentSecond / 60;
            int seconds = currentSecond % 60;
            std::cout << "⏱️  Position: " << minutes << ":" 
                      << (seconds < 10 ? "0" : "") << seconds << "\n";
            lastSecond = currentSecond;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n[EXIT] Shutting down...\n";
    session.reset();
    DestroyWindow(hwnd);
    return 0;
}
