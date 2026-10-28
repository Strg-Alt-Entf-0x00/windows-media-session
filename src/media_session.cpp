// ==============================================================================
// windows-media-session-0.8.0 - Public API Implementation
// ==============================================================================
// Description: Implementation of MediaSession public API (forwards to PIMPL)
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#include <media_session/media_session.h>
#include "media_session_impl.h"
#include "thumbnail_converter.h"

#include <winrt/base.h>  // For winrt::init_apartment
#include <iostream>      // For std::cout, std::cerr
#include <mutex>         // For std::once_flag, std::call_once

namespace windows_platform {

// ==============================================================================
// WinRT Apartment Initialization (Singleton)
// ==============================================================================
static bool ensureWinRTInitialized() {
    static std::once_flag initFlag;
    static bool initSuccess = false;
    
    std::call_once(initFlag, []() {
        try {
            winrt::init_apartment();
            std::cout << "[MediaSession] WinRT apartment initialized\n";
            initSuccess = true;
        } catch (const winrt::hresult_error& e) {
            // RPC_E_CHANGED_MODE (-2147417850) means COM already initialized in different mode
            // This is acceptable - WinRT can work with existing COM initialization
            if (e.code() == static_cast<int32_t>(0x80010106)) {  // RPC_E_CHANGED_MODE
                std::cout << "[MediaSession] WinRT: COM apartment already initialized (using existing)\n";
                initSuccess = true;
            } else {
                std::cerr << "[MediaSession] ERROR: WinRT initialization failed: " 
                          << e.code() << " - " << winrt::to_string(e.message()) << "\n";
                initSuccess = false;
            }
        } catch (const std::exception& e) {
            std::cerr << "[MediaSession] ERROR: WinRT initialization exception: " << e.what() << "\n";
            initSuccess = false;
        }
    });
    
    return initSuccess;
}

// ==============================================================================
// Factory Method
// ==============================================================================
std::unique_ptr<MediaSession> MediaSession::create(HWND hwnd) {
    try {
        // Validate HWND if provided
        // NULL/nullptr is OK (uses UWP GetForCurrentView mode)
        // But if HWND is provided, it must be a valid window
        if (hwnd != NULL && !IsWindow(hwnd)) {
            std::cerr << "[MediaSession] ERROR: Invalid HWND provided\n";
            return nullptr;  // Invalid HWND provided
        }
        
        std::cout << "[MediaSession] Creating with HWND: " << hwnd << "\n";
        
        // Ensure WinRT is initialized (singleton, thread-safe)
        if (!ensureWinRTInitialized()) {
            std::cerr << "[MediaSession] ERROR: WinRT initialization failed\n";
            return nullptr;
        }
        
        // Create MediaSession instance (unique_ptr with private constructor)
        auto session = std::unique_ptr<MediaSession>(new MediaSession());
        
        // Create PIMPL implementation
        session->impl_ = std::make_unique<Impl>(hwnd);
        
        // Initialize SystemMediaTransportControls
        auto error = session->impl_->initialize();
        if (!error.isSuccess()) {
            std::cerr << "[MediaSession] ERROR: Initialization failed: " 
                      << error.message << "\n";
            return nullptr;  // Initialization failed
        }
        
        std::cout << "[MediaSession] Successfully created\n";
        return session;
    }
    catch (const std::exception& e) {
        std::cerr << "[MediaSession] EXCEPTION: " << e.what() << "\n";
        return nullptr;  // Allocation or initialization failed
    }
    catch (...) {
        std::cerr << "[MediaSession] UNKNOWN EXCEPTION\n";
        return nullptr;  // Allocation or initialization failed
    }
}

// ==============================================================================
// Constructor & Destructor
// ==============================================================================
MediaSession::MediaSession() = default;

MediaSession::~MediaSession() = default;

// ==============================================================================
// Playback State Management
// ==============================================================================
ErrorInfo MediaSession::setPlaybackState(PlaybackState state) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Update internal state
    impl_->currentPlaybackState_ = state;
    
    // Forward to PIMPL: Update WinRT controls
    return Impl::wrapWinRTCall([this, state]() {
        auto winrtState = Impl::toWinRTPlaybackStatus(state);
        impl_->controls_.PlaybackStatus(winrtState);
    });
}

PlaybackState MediaSession::getPlaybackState() const noexcept {
    if (!impl_) {
        return PlaybackState::Closed;
    }
    return impl_->currentPlaybackState_;
}

ErrorInfo MediaSession::setEnabled(bool enabled) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Enable/disable WinRT controls
    return Impl::wrapWinRTCall([this, enabled]() {
        impl_->controls_.IsEnabled(enabled);
    });
}

bool MediaSession::isEnabled() const noexcept {
    if (!impl_) {
        return false;
    }
    
    try {
        return impl_->controls_.IsEnabled();
    }
    catch (...) {
        return false;
    }
}

// ==============================================================================
// Metadata Management
// ==============================================================================
ErrorInfo MediaSession::setMetadata(const Metadata& meta) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    if (!meta.title || meta.title[0] == '\0') {
        return {ErrorCode::InvalidParameter, "Metadata title is required"};
    }
    
    // Forward to PIMPL: Update WinRT metadata
    return Impl::wrapWinRTCall([this, &meta]() {
        // Set media type first (required before setting properties)
        impl_->updater_.Type(winrt::Windows::Media::MediaPlaybackType::Music);
        
        auto musicProps = impl_->updater_.MusicProperties();
        
        // Set title (required)
        musicProps.Title(winrt::to_hstring(meta.title));
        
        // Set artist (optional)
        if (meta.artist && meta.artist[0] != '\0') {
            musicProps.Artist(winrt::to_hstring(meta.artist));
        } else {
            musicProps.Artist(L"");
        }
        
        // Set album (optional) - use AlbumTitle, not AlbumArtist
        if (meta.album && meta.album[0] != '\0') {
            musicProps.AlbumTitle(winrt::to_hstring(meta.album));
        } else {
            musicProps.AlbumTitle(L"");
        }
        
        // Set track number (optional)
        if (meta.trackNumber > 0) {
            musicProps.TrackNumber(meta.trackNumber);
        }
        
        // Set app media ID (optional)
        if (meta.appMediaId && meta.appMediaId[0] != '\0') {
            impl_->updater_.AppMediaId(winrt::to_hstring(meta.appMediaId));
        }
        
        // Apply changes (this pushes to system UI)
        impl_->updater_.Update();
    });
}

ErrorInfo MediaSession::clearMetadata() noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Clear WinRT metadata
    return Impl::wrapWinRTCall([this]() {
        auto musicProps = impl_->updater_.MusicProperties();
        musicProps.Title(L"");
        musicProps.Artist(L"");
        musicProps.AlbumArtist(L"");
        impl_->updater_.Update();
    });
}

// ==============================================================================
// Thumbnail Management
// ==============================================================================
ErrorInfo MediaSession::setThumbnail(const uint8_t* data, size_t size) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    if (!data || size == 0) {
        return {ErrorCode::InvalidParameter, "Invalid thumbnail data"};
    }
    
    // Convert image data to WinRT stream
    winrt::Windows::Storage::Streams::RandomAccessStreamReference streamRef{nullptr};
    auto error = ThumbnailConverter::createStreamFromMemory(data, size, streamRef);
    
    if (!error.isSuccess()) {
        return error;
    }
    
    // Forward to PIMPL: Set thumbnail in DisplayUpdater
    return Impl::wrapWinRTCall([this, streamRef]() {
        impl_->updater_.Thumbnail(streamRef);
        impl_->updater_.Update();
    });
}

ErrorInfo MediaSession::clearThumbnail() noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Clear thumbnail
    return Impl::wrapWinRTCall([this]() {
        impl_->updater_.Thumbnail(nullptr);
        impl_->updater_.Update();
    });
}

// ==============================================================================
// Timeline Management
// ==============================================================================
ErrorInfo MediaSession::setPosition(std::chrono::milliseconds position,
                                    std::chrono::milliseconds duration) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Update WinRT timeline
    return Impl::wrapWinRTCall([this, position, duration]() {
        auto timelineProps = impl_->updater_.MusicProperties();
        
        // Note: SystemMediaTransportControls uses TimeSpan (100-nanosecond units)
        // std::chrono::milliseconds → Windows::Foundation::TimeSpan
        winrt::Windows::Foundation::TimeSpan positionTimeSpan{
            std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 10000000>>>(position).count()
        };
        
        winrt::Windows::Foundation::TimeSpan durationTimeSpan{
            std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 10000000>>>(duration).count()
        };
        
        // Update timeline properties (not directly exposed, requires TimelineProperties)
        // For now, we store duration in MusicProperties - position updates not directly supported
        // This is a known limitation of SystemMediaTransportControls
        (void)positionTimeSpan;  // Position updates not supported by API
        (void)durationTimeSpan;  // Duration not directly settable
        
        // Note: Full timeline support requires Windows.Media.Playback.MediaPlayer integration
        // which is more complex and outside current scope
    });
}

ErrorInfo MediaSession::enableSeekbar(bool enable) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Note: SystemMediaTransportControls does not have direct seekbar enable/disable
    // Seekbar visibility is controlled by whether position/duration are set
    // This is a no-op for current API version
    (void)enable;
    return kSuccess;
}

// ==============================================================================
// Button Availability
// ==============================================================================
ErrorInfo MediaSession::setButtonEnabled(MediaButton button, bool enabled) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Update WinRT button availability
    return Impl::wrapWinRTCall([this, button, enabled]() {
        switch (button) {
            case MediaButton::Play:
                impl_->controls_.IsPlayEnabled(enabled);
                break;
            case MediaButton::Pause:
                impl_->controls_.IsPauseEnabled(enabled);
                break;
            case MediaButton::Stop:
                impl_->controls_.IsStopEnabled(enabled);
                break;
            case MediaButton::Next:
                impl_->controls_.IsNextEnabled(enabled);
                break;
            case MediaButton::Previous:
                impl_->controls_.IsPreviousEnabled(enabled);
                break;
            case MediaButton::FastForward:
                impl_->controls_.IsFastForwardEnabled(enabled);
                break;
            case MediaButton::Rewind:
                impl_->controls_.IsRewindEnabled(enabled);
                break;
            default:
                throw winrt::hresult_invalid_argument(L"Invalid MediaButton");
        }
    });
}

// ==============================================================================
// Event Callbacks
// ==============================================================================
void MediaSession::onPlay(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->playCallback_ = std::move(callback);
    }
}

void MediaSession::onPause(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->pauseCallback_ = std::move(callback);
    }
}

void MediaSession::onStop(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->stopCallback_ = std::move(callback);
    }
}

void MediaSession::onNext(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->nextCallback_ = std::move(callback);
    }
}

void MediaSession::onPrevious(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->previousCallback_ = std::move(callback);
    }
}

void MediaSession::onFastForward(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->fastForwardCallback_ = std::move(callback);
    }
}

void MediaSession::onRewind(std::function<void()> callback) noexcept {
    if (impl_) {
        impl_->rewindCallback_ = std::move(callback);
    }
}

void MediaSession::onSeekRequested(std::function<void(std::chrono::milliseconds)> callback) noexcept {
    if (impl_) {
        impl_->seekRequestedCallback_ = std::move(callback);
    }
}

void MediaSession::onShuffleChangeRequested(std::function<void(bool)> callback) noexcept {
    if (impl_) {
        impl_->shuffleChangeRequestedCallback_ = std::move(callback);
    }
}

void MediaSession::onRepeatModeChangeRequested(std::function<void(RepeatMode)> callback) noexcept {
    if (impl_) {
        impl_->repeatModeChangeRequestedCallback_ = std::move(callback);
    }
}

// ==============================================================================
// Timeline Management (NEW - Full Implementation)
// ==============================================================================
ErrorInfo MediaSession::updateTimeline(std::chrono::milliseconds position,
                                       std::chrono::milliseconds duration) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    // Forward to PIMPL: Update WinRT timeline with SystemMediaTransportControlsTimelineProperties
    return Impl::wrapWinRTCall([this, position, duration]() {
        // Create timeline properties
        winrt::Windows::Media::SystemMediaTransportControlsTimelineProperties timelineProps;
        
        // Convert milliseconds to TimeSpan (100-nanosecond units)
        auto toTimeSpan = [](std::chrono::milliseconds ms) {
            return winrt::Windows::Foundation::TimeSpan{
                std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 10000000>>>(ms).count()
            };
        };
        
        timelineProps.StartTime(toTimeSpan(std::chrono::milliseconds{0}));
        timelineProps.EndTime(toTimeSpan(duration));
        timelineProps.Position(toTimeSpan(position));
        timelineProps.MinSeekTime(toTimeSpan(std::chrono::milliseconds{0}));
        timelineProps.MaxSeekTime(toTimeSpan(duration));
        
        // Update timeline
        impl_->controls_.UpdateTimelineProperties(timelineProps);
    });
}

// ==============================================================================
// Shuffle & Repeat
// ==============================================================================
ErrorInfo MediaSession::setShuffleEnabled(bool enabled) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    impl_->shuffleEnabled_ = enabled;
    
    return Impl::wrapWinRTCall([this, enabled]() {
        impl_->controls_.ShuffleEnabled(enabled);
    });
}

bool MediaSession::isShuffleEnabled() const noexcept {
    if (!impl_) {
        return false;
    }
    return impl_->shuffleEnabled_;
}

ErrorInfo MediaSession::setRepeatMode(RepeatMode mode) noexcept {
    if (!impl_) {
        return {ErrorCode::NotInitialized, "MediaSession not initialized"};
    }
    
    impl_->repeatMode_ = mode;
    
    return Impl::wrapWinRTCall([this, mode]() {
        auto winrtMode = Impl::toWinRTRepeatMode(mode);
        impl_->controls_.AutoRepeatMode(winrtMode);
    });
}

RepeatMode MediaSession::getRepeatMode() const noexcept {
    if (!impl_) {
        return RepeatMode::None;
    }
    return impl_->repeatMode_;
}

} // namespace windows_platform

