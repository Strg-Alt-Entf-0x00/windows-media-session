// ==============================================================================
// windows-media-session-0.8.0 - PIMPL Implementation
// ==============================================================================
// Description: WinRT implementation (hidden from public API)
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#include "media_session_impl.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winerror.h>  // For E_FAIL and other HRESULT codes

namespace windows_platform {

// ==============================================================================
// Constructor & Destructor
// ==============================================================================
MediaSession::Impl::Impl(HWND hwnd)
    : hwnd_(hwnd) {
    // Initialization happens in initialize()
}

MediaSession::Impl::~Impl() {
    // Cleanup WinRT event handlers
    if (controls_) {
        if (buttonPressedToken_) {
            controls_.ButtonPressed(buttonPressedToken_);
        }
        if (shuffleChangeRequestedToken_) {
            controls_.ShuffleEnabledChangeRequested(shuffleChangeRequestedToken_);
        }
        if (repeatModeChangeRequestedToken_) {
            controls_.AutoRepeatModeChangeRequested(repeatModeChangeRequestedToken_);
        }
        if (seekRequestedToken_) {
            controls_.PlaybackPositionChangeRequested(seekRequestedToken_);
        }
    }
}

// ==============================================================================
// Initialization
// ==============================================================================
ErrorInfo MediaSession::Impl::initialize() {
    return wrapWinRTCall([this]() {
        // ======================================================================
        // MODERN APPROACH: Use ISystemMediaTransportControlsInterop
        // ======================================================================
        // This is the OFFICIAL way for Desktop apps to get SystemMediaTransportControls
        // Works for Win32, WPF, Windows Forms, Qt, etc.
        //
        // Reference: https://learn.microsoft.com/en-us/windows/win32/api/systemmediatransportcontrolsinterop/
        
        if (hwnd_) {
            // Desktop app with HWND: Use Interop interface (RECOMMENDED)
            auto interop = winrt::get_activation_factory<
                winrt::Windows::Media::SystemMediaTransportControls,
                ISystemMediaTransportControlsInterop
            >();
            
            // Get SystemMediaTransportControls for specific window
            winrt::Windows::Media::SystemMediaTransportControls tempControls{nullptr};
            winrt::check_hresult(interop->GetForWindow(
                hwnd_,
                winrt::guid_of<winrt::Windows::Media::SystemMediaTransportControls>(),
                winrt::put_abi(tempControls)
            ));
            
            controls_ = tempControls;
        }
        else {
            // UWP app: Use GetForCurrentView()
            controls_ = winrt::Windows::Media::SystemMediaTransportControls::GetForCurrentView();
        }
        
        if (!controls_) {
            throw winrt::hresult_error(E_FAIL, L"Failed to get SystemMediaTransportControls");
        }
        
        // Get DisplayUpdater for metadata
        updater_ = controls_.DisplayUpdater();
        
        if (!updater_) {
            throw winrt::hresult_error(E_FAIL, L"Failed to get DisplayUpdater");
        }
        
        // DON'T enable controls immediately! Let app control when to show them.
        // BEST PRACTICE: Enable only when playback starts, disable when stopped.
        // This matches Firefox, Spotify, VLC behavior.
        controls_.IsEnabled(false);  // Start disabled
        
        // Configure which buttons are available
        controls_.IsPlayEnabled(true);
        controls_.IsPauseEnabled(true);
        controls_.IsStopEnabled(true);
        controls_.IsNextEnabled(true);
        controls_.IsPreviousEnabled(true);
        
        // Register event handlers for button presses
        buttonPressedToken_ = controls_.ButtonPressed([this](
            winrt::Windows::Media::SystemMediaTransportControls const& sender,
            winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs const& args
        ) {
            onButtonPressed(sender, args);
        });
        
        // Register event handler for shuffle change requests
        shuffleChangeRequestedToken_ = controls_.ShuffleEnabledChangeRequested([this](
            winrt::Windows::Media::SystemMediaTransportControls const& sender,
            winrt::Windows::Media::ShuffleEnabledChangeRequestedEventArgs const& args
        ) {
            onShuffleChangeRequested(sender, args);
        });
        
        // Register event handler for repeat mode change requests
        repeatModeChangeRequestedToken_ = controls_.AutoRepeatModeChangeRequested([this](
            winrt::Windows::Media::SystemMediaTransportControls const& sender,
            winrt::Windows::Media::AutoRepeatModeChangeRequestedEventArgs const& args
        ) {
            onRepeatModeChangeRequested(sender, args);
        });
        
        // Register event handler for seek requests
        seekRequestedToken_ = controls_.PlaybackPositionChangeRequested([this](
            winrt::Windows::Media::SystemMediaTransportControls const& sender,
            winrt::Windows::Media::PlaybackPositionChangeRequestedEventArgs const& args
        ) {
            onSeekRequested(sender, args);
        });
    });
}

// ==============================================================================
// Event Handler: Button Pressed
// ==============================================================================
void MediaSession::Impl::onButtonPressed(
    winrt::Windows::Media::SystemMediaTransportControls const&,
    winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs const& args
) {
    using winrt::Windows::Media::SystemMediaTransportControlsButton;
    
    // Determine which button was pressed and invoke corresponding callback
    switch (args.Button()) {
        case SystemMediaTransportControlsButton::Play:
            if (playCallback_) {
                playCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::Pause:
            if (pauseCallback_) {
                pauseCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::Stop:
            if (stopCallback_) {
                stopCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::Next:
            if (nextCallback_) {
                nextCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::Previous:
            if (previousCallback_) {
                previousCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::FastForward:
            if (fastForwardCallback_) {
                fastForwardCallback_();
            }
            break;
            
        case SystemMediaTransportControlsButton::Rewind:
            if (rewindCallback_) {
                rewindCallback_();
            }
            break;
            
        default:
            // Unsupported button
            break;
    }
}

// ==============================================================================
// Helper Methods
// ==============================================================================
winrt::Windows::Media::MediaPlaybackStatus 
MediaSession::Impl::toWinRTPlaybackStatus(PlaybackState state) noexcept {
    using winrt::Windows::Media::MediaPlaybackStatus;
    
    switch (state) {
        case PlaybackState::Playing:
            return MediaPlaybackStatus::Playing;
        case PlaybackState::Paused:
            return MediaPlaybackStatus::Paused;
        case PlaybackState::Stopped:
            return MediaPlaybackStatus::Stopped;
        case PlaybackState::Closed:
        default:
            return MediaPlaybackStatus::Closed;
    }
}

ErrorInfo MediaSession::Impl::wrapWinRTCall(std::function<void()> action) noexcept {
    try {
        action();
        return kSuccess;
    }
    catch (const winrt::hresult_error&) {
        return {ErrorCode::WinRTFailed, "WinRT API call failed"};
    }
    catch (const std::bad_alloc&) {
        return {ErrorCode::OutOfMemory, "Out of memory"};
    }
    catch (...) {
        return {ErrorCode::UnknownError, "Unknown error occurred"};
    }
}

// ==============================================================================
// New Event Handlers
// ==============================================================================
void MediaSession::Impl::onShuffleChangeRequested(
    winrt::Windows::Media::SystemMediaTransportControls const&,
    winrt::Windows::Media::ShuffleEnabledChangeRequestedEventArgs const& args
) {
    if (shuffleChangeRequestedCallback_) {
        shuffleChangeRequestedCallback_(args.RequestedShuffleEnabled());
    }
}

void MediaSession::Impl::onRepeatModeChangeRequested(
    winrt::Windows::Media::SystemMediaTransportControls const&,
    winrt::Windows::Media::AutoRepeatModeChangeRequestedEventArgs const& args
) {
    if (repeatModeChangeRequestedCallback_) {
        auto mode = fromWinRTRepeatMode(args.RequestedAutoRepeatMode());
        repeatModeChangeRequestedCallback_(mode);
    }
}

void MediaSession::Impl::onSeekRequested(
    winrt::Windows::Media::SystemMediaTransportControls const&,
    winrt::Windows::Media::PlaybackPositionChangeRequestedEventArgs const& args
) {
    if (seekRequestedCallback_) {
        auto requestedPosition = args.RequestedPlaybackPosition();
        // Convert TimeSpan (100-nanosecond units) to milliseconds
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::duration<int64_t, std::ratio<1, 10000000>>(requestedPosition.count())
        );
        seekRequestedCallback_(ms);
    }
}

// ==============================================================================
// Helper Methods (NEW)
// ==============================================================================
winrt::Windows::Media::MediaPlaybackAutoRepeatMode
MediaSession::Impl::toWinRTRepeatMode(RepeatMode mode) noexcept {
    using winrt::Windows::Media::MediaPlaybackAutoRepeatMode;
    
    switch (mode) {
        case RepeatMode::None:
            return MediaPlaybackAutoRepeatMode::None;
        case RepeatMode::Track:
            return MediaPlaybackAutoRepeatMode::Track;
        case RepeatMode::List:
            return MediaPlaybackAutoRepeatMode::List;
        default:
            return MediaPlaybackAutoRepeatMode::None;
    }
}

RepeatMode
MediaSession::Impl::fromWinRTRepeatMode(winrt::Windows::Media::MediaPlaybackAutoRepeatMode mode) noexcept {
    using winrt::Windows::Media::MediaPlaybackAutoRepeatMode;
    
    switch (mode) {
        case MediaPlaybackAutoRepeatMode::None:
            return RepeatMode::None;
        case MediaPlaybackAutoRepeatMode::Track:
            return RepeatMode::Track;
        case MediaPlaybackAutoRepeatMode::List:
            return RepeatMode::List;
        default:
            return RepeatMode::None;
    }
}

} // namespace windows_platform

