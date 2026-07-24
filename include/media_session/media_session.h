// ==============================================================================
// windows-media-session-0.8.0 - Main Public API
// ==============================================================================
// Description: Production-grade Windows Media Session integration
// License:     MIT
// Created:     2026-06-22
// ==============================================================================
// 
// This library provides framework-agnostic C++20 interface to Windows
// SystemMediaTransportControls (Lock Screen, Quick Settings, Hardware Keys).
// 
// Features:
//   - Playback state management (Playing, Paused, Stopped)
//   - Metadata display (Title, Artist, Album)
//   - Thumbnail support (PNG, JPEG, BMP)
//   - Timeline/position tracking
//   - Media button event callbacks
//   - Exception-free public API (ErrorInfo-based)
// 
// Requirements:
//   - Windows 10 Build 14393+ (Anniversary Update)
//   - C++20 compiler
//   - MSVC 2022+ or Clang 14+
// 
// Example:
//   #include <media_session/media_session.h>
//   
//   auto session = windows_platform::MediaSession::create(hwnd);
//   
//   windows_platform::Metadata meta;
//   meta.title = "Test Song";
//   meta.artist = "Test Artist";
//   session->setMetadata(meta);
//   
//   session->onPlay([] { printf("Play pressed\n"); });
//   session->setPlaybackState(windows_platform::PlaybackState::Playing);
// 
// ==============================================================================

#pragma once

#include "error.h"
#include "types.h"

#include <functional>
#include <memory>

// Forward declare Windows types (avoid including <windows.h> in public header)
#ifndef _WINDEF_
struct HWND__;
typedef HWND__* HWND;
#endif

namespace windows_platform {

// ==============================================================================
// MediaSession Class
// ==============================================================================
/**
 * @brief Windows Media Session integration
 * 
 * Provides access to Windows SystemMediaTransportControls for displaying
 * playback information in system UI (Lock Screen, Quick Settings, etc.).
 * 
 * Thread Safety:
 *   - All public methods are thread-safe
 *   - Callbacks are invoked on the application thread
 * 
 * Lifecycle:
 *   - Create via factory method: create()
 *   - Automatically cleaned up via RAII (unique_ptr)
 *   - Must keep instance alive while using media controls
 * 
 * Error Handling:
 *   - All methods return ErrorInfo (no exceptions in public API)
 *   - Check error.isSuccess() before proceeding
 * 
 * Example:
 * @code
 *   auto session = MediaSession::create(hwnd);
 *   if (!session) {
 *       fprintf(stderr, "Failed to create MediaSession\n");
 *       return;
 *   }
 *   
 *   auto error = session->setPlaybackState(PlaybackState::Playing);
 *   if (!error.isSuccess()) {
 *       fprintf(stderr, "Error: %s\n", error.message);
 *   }
 * @endcode
 */
class MediaSession {
public:
    // ==========================================================================
    // Factory Method
    // ==========================================================================
    /**
     * @brief Create a new MediaSession instance
     * 
     * @param hwnd Window handle (HWND) for desktop applications.
     *             Pass nullptr for UWP apps (uses GetForCurrentView).
     * 
     * @return std::unique_ptr<MediaSession> on success, nullptr on failure
     * 
     * Failure reasons:
     *   - Unsupported OS version (< Windows 10 Build 14393)
     *   - SystemMediaTransportControls initialization failed
     *   - Out of memory
     * 
     * Example:
     * @code
     *   HWND hwnd = GetActiveWindow();
     *   auto session = MediaSession::create(hwnd);
     *   if (!session) {
     *       // Handle error
     *   }
     * @endcode
     */
    [[nodiscard]] static std::unique_ptr<MediaSession> create(HWND hwnd);
    
    // ==========================================================================
    // Playback State Management
    // ==========================================================================
    /**
     * @brief Set current playback state
     * 
     * Updates the playback status shown in Windows media controls.
     * 
     * @param state New playback state (Playing, Paused, Stopped, Closed)
     * @return ErrorInfo (Success or error details)
     * 
     * Example:
     * @code
     *   session->setPlaybackState(PlaybackState::Playing);
     * @endcode
     */
    [[nodiscard]] ErrorInfo setPlaybackState(PlaybackState state) noexcept;
    
    /**
     * @brief Get current playback state
     * 
     * @return Current PlaybackState
     */
    [[nodiscard]] PlaybackState getPlaybackState() const noexcept;
    
    /**
     * @brief Enable or disable media controls visibility
     * 
     * Controls whether the media controls are visible in Windows SMTC 
     * (Quick Settings, Lock Screen, etc.).
     * 
     * BEST PRACTICE:
     * - Call setEnabled(true) when playback STARTS
     * - Call setEnabled(false) when playback STOPS
     * 
     * This matches behavior of professional apps (Firefox, Spotify, VLC) where
     * controls only appear when media is actively playing.
     * 
     * @param enabled true to show controls, false to hide them
     * @return ErrorInfo (Success or error details)
     * 
     * Example:
     * @code
     *   // When starting playback
     *   session->setEnabled(true);
     *   session->setPlaybackState(PlaybackState::Playing);
     *   
     *   // When stopping playback
     *   session->setPlaybackState(PlaybackState::Stopped);
     *   session->setEnabled(false);
     * @endcode
     */
    [[nodiscard]] ErrorInfo setEnabled(bool enabled) noexcept;
    
    /**
     * @brief Get current enabled state
     * 
     * @return true if controls are visible, false if hidden
     */
    [[nodiscard]] bool isEnabled() const noexcept;
    
    // ==========================================================================
    // Metadata Management
    // ==========================================================================
    /**
     * @brief Set media metadata (title, artist, album, duration)
     * 
     * Updates the track information displayed in Windows media controls.
     * All string pointers must remain valid for the duration of this call.
     * 
     * @param meta Metadata structure (title required, others optional)
     * @return ErrorInfo (Success or error details)
     * 
     * Example:
     * @code
     *   Metadata meta;
     *   meta.title = "Hotel California";
     *   meta.artist = "Eagles";
     *   meta.album = "Hotel California";
     *   meta.duration = std::chrono::minutes(6) + std::chrono::seconds(30);
     *   session->setMetadata(meta);
     * @endcode
     */
    [[nodiscard]] ErrorInfo setMetadata(const Metadata& meta) noexcept;
    
    /**
     * @brief Clear all metadata
     * 
     * Removes title, artist, album from media controls.
     * 
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo clearMetadata() noexcept;
    
    // ==========================================================================
    // Thumbnail Management
    // ==========================================================================
    /**
     * @brief Set album art / thumbnail
     * 
     * Displays album art in Windows media controls (Lock Screen, Quick Settings).
     * 
     * Supported formats: PNG, JPEG, BMP
     * Recommended: PNG 300x300 pixels
     * Maximum: 1920x1920 pixels
     * 
     * @param data Raw image data (PNG/JPEG/BMP bytes)
     * @param size Size in bytes
     * @return ErrorInfo (Success or InvalidImageFormat or other error)
     * 
     * Example:
     * @code
     *   std::vector<uint8_t> png = loadImageFile("cover.png");
     *   session->setThumbnail(png.data(), png.size());
     * @endcode
     */
    [[nodiscard]] ErrorInfo setThumbnail(const uint8_t* data, size_t size) noexcept;
    
    /**
     * @brief Clear thumbnail
     * 
     * Removes album art from media controls.
     * 
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo clearThumbnail() noexcept;
    
    // ==========================================================================
    // Timeline Management
    // ==========================================================================
    /**
     * @brief Update timeline with position and duration
     * 
     * Updates the timeline/seekbar in Windows media controls.
     * This method uses SystemMediaTransportControlsTimelineProperties
     * for proper seekbar display and position tracking.
     * 
     * @param position Current playback position
     * @param duration Total media duration
     * @return ErrorInfo (Success or error details)
     * 
     * Example:
     * @code
     *   session->updateTimeline(std::chrono::seconds(90), 
     *                           std::chrono::minutes(6) + std::chrono::seconds(30));
     * @endcode
     */
    [[nodiscard]] ErrorInfo updateTimeline(std::chrono::milliseconds position,
                                           std::chrono::milliseconds duration) noexcept;
    
    /**
     * @brief Set playback position and duration (deprecated - use updateTimeline)
     * 
     * @deprecated Use updateTimeline() instead for proper timeline support
     */
    [[nodiscard]] ErrorInfo setPosition(std::chrono::milliseconds position,
                                        std::chrono::milliseconds duration) noexcept;
    
    /**
     * @brief Enable or disable seekbar
     * 
     * Controls whether the timeline is shown in media controls.
     * When enabled, users can see and interact with the seekbar.
     * 
     * @param enable true to show seekbar, false to hide
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo enableSeekbar(bool enable) noexcept;
    
    // ==========================================================================
    // Button Availability
    // ==========================================================================
    /**
     * @brief Enable or disable a media button
     * 
     * Controls which buttons are shown/enabled in media controls.
     * Example: Disable "Previous" button when at first track.
     * 
     * @param button MediaButton to configure
     * @param enabled true to enable, false to disable
     * @return ErrorInfo (Success or error details)
     * 
     * Example:
     * @code
     *   session->setButtonEnabled(MediaButton::Previous, false);
     * @endcode
     */
    [[nodiscard]] ErrorInfo setButtonEnabled(MediaButton button, bool enabled) noexcept;
    
    // ==========================================================================
    // Event Callbacks
    // ==========================================================================
    /**
     * @brief Register callback for Play button press
     * 
     * Callback is invoked on application thread when user presses Play
     * button in media controls or via hardware keys.
     * 
     * @param callback Function to call (can be lambda)
     * 
     * Example:
     * @code
     *   session->onPlay([] { printf("Play pressed\n"); });
     * @endcode
     */
    void onPlay(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for Pause button press
     * @param callback Function to call when Pause is pressed
     */
    void onPause(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for Stop button press
     * @param callback Function to call when Stop is pressed
     */
    void onStop(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for Next button press
     * @param callback Function to call when Next is pressed
     */
    void onNext(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for Previous button press
     * @param callback Function to call when Previous is pressed
     */
    void onPrevious(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for FastForward button press
     * @param callback Function to call when FastForward is pressed
     */
    void onFastForward(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for Rewind button press
     * @param callback Function to call when Rewind is pressed
     */
    void onRewind(std::function<void()> callback) noexcept;
    
    /**
     * @brief Register callback for seek position change request
     * @param callback Function to call with requested position in milliseconds
     */
    void onSeekRequested(std::function<void(std::chrono::milliseconds)> callback) noexcept;
    
    // ==========================================================================
    // Shuffle & Repeat
    // ==========================================================================
    /**
     * @brief Enable or disable shuffle mode
     * 
     * @param enabled true to enable shuffle, false to disable
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo setShuffleEnabled(bool enabled) noexcept;
    
    /**
     * @brief Get current shuffle state
     * @return true if shuffle is enabled
     */
    [[nodiscard]] bool isShuffleEnabled() const noexcept;
    
    /**
     * @brief Register callback for shuffle change request
     * @param callback Function to call with requested shuffle state
     */
    void onShuffleChangeRequested(std::function<void(bool)> callback) noexcept;
    
    /**
     * @brief Set repeat mode
     * 
     * @param mode Repeat mode (None/Track/List)
     * @return ErrorInfo (Success or error details)
     */
    [[nodiscard]] ErrorInfo setRepeatMode(RepeatMode mode) noexcept;
    
    /**
     * @brief Get current repeat mode
     * @return Current RepeatMode
     */
    [[nodiscard]] RepeatMode getRepeatMode() const noexcept;
    
    /**
     * @brief Register callback for repeat mode change request
     * @param callback Function to call with requested repeat mode
     */
    void onRepeatModeChangeRequested(std::function<void(RepeatMode)> callback) noexcept;
    
    // ==========================================================================
    // Lifecycle
    // ==========================================================================
    /**
     * @brief Destructor (cleanup WinRT resources)
     */
    ~MediaSession();
    
    // Delete copy/move (PIMPL with unique ownership)
    MediaSession(const MediaSession&) = delete;
    MediaSession& operator=(const MediaSession&) = delete;
    MediaSession(MediaSession&&) = delete;
    MediaSession& operator=(MediaSession&&) = delete;

private:
    // Constructor is private (use factory create())
    MediaSession();
    
    // PIMPL: Hide WinRT implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace windows_platform
