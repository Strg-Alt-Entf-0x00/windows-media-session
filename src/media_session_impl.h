// ==============================================================================
// windows-media-session-0.8.0 - PIMPL Implementation Header
// ==============================================================================
// Description: Private implementation (hides WinRT complexity)
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#pragma once

#include <media_session/media_session.h>
#include <media_session/types.h>
#include <media_session/error.h>

#include <functional>
#include <memory>

// WinRT Headers (only in implementation, NOT in public API)
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Storage.Streams.h>

// Windows headers for HWND and COM interop
#include <windows.h>
#include <systemmediatransportcontrolsinterop.h>  // ISystemMediaTransportControlsInterop

namespace windows_platform {

// ==============================================================================
// MediaSession PIMPL Implementation
// ==============================================================================
/**
 * @brief Private implementation of MediaSession (PIMPL pattern)
 * 
 * This struct contains all WinRT-specific code, keeping the public API
 * clean and free from WinRT dependencies.
 * 
 * Thread Safety:
 *   - All WinRT calls must be made on UI thread
 *   - Callbacks dispatched via Windows message loop
 */
struct MediaSession::Impl {
    // ==========================================================================
    // Constructor & Destructor
    // ==========================================================================
    /**
     * @brief Initialize MediaSession implementation
     * @param hwnd Window handle for desktop apps (nullptr for UWP)
     */
    explicit Impl(HWND hwnd);
    
    /**
     * @brief Cleanup WinRT resources
     */
    ~Impl();
    
    // ==========================================================================
    // Initialization
    // ==========================================================================
    /**
     * @brief Initialize SystemMediaTransportControls
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo initialize();
    
    // ==========================================================================
    // State Management
    // ==========================================================================
    PlaybackState currentPlaybackState_{PlaybackState::Closed};
    bool shuffleEnabled_{false};
    RepeatMode repeatMode_{RepeatMode::None};
    bool seekbarEnabled_{false};
    
    // ==========================================================================
    // WinRT Members (SystemMediaTransportControls)
    // ==========================================================================
    winrt::Windows::Media::SystemMediaTransportControls controls_{nullptr};
    winrt::Windows::Media::SystemMediaTransportControlsDisplayUpdater updater_{nullptr};
    
    // ==========================================================================
    // Event Token Storage (for unregistering callbacks)
    // ==========================================================================
    winrt::event_token buttonPressedToken_{};
    winrt::event_token shuffleChangeRequestedToken_{};
    winrt::event_token repeatModeChangeRequestedToken_{};
    winrt::event_token seekRequestedToken_{};
    
    // ==========================================================================
    // User Callbacks
    // ==========================================================================
    std::function<void()> playCallback_;
    std::function<void()> pauseCallback_;
    std::function<void()> stopCallback_;
    std::function<void()> nextCallback_;
    std::function<void()> previousCallback_;
    std::function<void()> fastForwardCallback_;
    std::function<void()> rewindCallback_;
    std::function<void(std::chrono::milliseconds)> seekRequestedCallback_;
    std::function<void(bool)> shuffleChangeRequestedCallback_;
    std::function<void(RepeatMode)> repeatModeChangeRequestedCallback_;
    
    // ==========================================================================
    // Window Handle (for message-based callback dispatch)
    // ==========================================================================
    HWND hwnd_{nullptr};
    
    // ==========================================================================
    // Helper Methods
    // ==========================================================================
    /**
     * @brief Convert PlaybackState enum to WinRT MediaPlaybackStatus
     */
    [[nodiscard]] static winrt::Windows::Media::MediaPlaybackStatus 
        toWinRTPlaybackStatus(PlaybackState state) noexcept;
    
    /**
     * @brief Convert RepeatMode enum to WinRT MediaPlaybackAutoRepeatMode
     */
    [[nodiscard]] static winrt::Windows::Media::MediaPlaybackAutoRepeatMode
        toWinRTRepeatMode(RepeatMode mode) noexcept;
    
    /**
     * @brief Convert WinRT MediaPlaybackAutoRepeatMode to RepeatMode enum
     */
    [[nodiscard]] static RepeatMode
        fromWinRTRepeatMode(winrt::Windows::Media::MediaPlaybackAutoRepeatMode mode) noexcept;
    
    /**
     * @brief Wrap WinRT exceptions into ErrorInfo
     */
    [[nodiscard]] static ErrorInfo wrapWinRTCall(std::function<void()> action) noexcept;
    
    /**
     * @brief Event handler for button presses
     */
    void onButtonPressed(
        winrt::Windows::Media::SystemMediaTransportControls const& sender,
        winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs const& args
    );
    
    /**
     * @brief Event handler for shuffle change requests
     */
    void onShuffleChangeRequested(
        winrt::Windows::Media::SystemMediaTransportControls const& sender,
        winrt::Windows::Media::ShuffleEnabledChangeRequestedEventArgs const& args
    );
    
    /**
     * @brief Event handler for repeat mode change requests
     */
    void onRepeatModeChangeRequested(
        winrt::Windows::Media::SystemMediaTransportControls const& sender,
        winrt::Windows::Media::AutoRepeatModeChangeRequestedEventArgs const& args
    );
    
    /**
     * @brief Event handler for seek requests
     */
    void onSeekRequested(
        winrt::Windows::Media::SystemMediaTransportControls const& sender,
        winrt::Windows::Media::PlaybackPositionChangeRequestedEventArgs const& args
    );
};

} // namespace windows_platform
