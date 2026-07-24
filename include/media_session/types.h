// ==============================================================================
// windows-media-session-0.8.0 - Type Definitions
// ==============================================================================
// Description: Enums, structs, and type aliases for MediaSession API
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace windows_platform {

// ==============================================================================
// Playback State Enumeration
// ==============================================================================
/**
 * @brief Playback state exposed to Windows system media controls
 * 
 * Maps to Windows::Media::MediaPlaybackStatus
 */
enum class PlaybackState {
    /// Player is closed (no media loaded)
    Closed = 0,
    
    /// Media is currently playing
    Playing = 1,
    
    /// Media is paused
    Paused = 2,
    
    /// Media is stopped (loaded but not playing)
    Stopped = 3
};

// ==============================================================================
// Media Button Enumeration
// ==============================================================================
/**
 * @brief Media control buttons that can be enabled/disabled
 * 
 * Maps to Windows::Media::SystemMediaTransportControlsButton
 */
enum class MediaButton {
    Play,
    Pause,
    Stop,
    Next,
    Previous,
    FastForward,
    Rewind
};

// ==============================================================================
// Repeat Mode Enumeration
// ==============================================================================
/**
 * @brief Repeat mode for playback
 * 
 * Maps to Windows::Media::MediaPlaybackAutoRepeatMode
 */
enum class RepeatMode {
    /// No repeat
    None = 0,
    
    /// Repeat current track
    Track = 1,
    
    /// Repeat playlist/album
    List = 2
};

// ==============================================================================
// Metadata Structure
// ==============================================================================
/**
 * @brief Media metadata displayed in system media controls
 * 
 * All string fields must be UTF-8 encoded and null-terminated.
 * Lifetime of string pointers must outlive the setMetadata() call.
 * 
 * Example:
 * @code
 *   Metadata meta;
 *   meta.title = "Hotel California";
 *   meta.artist = "Eagles";
 *   meta.album = "Hotel California";
 *   meta.duration = std::chrono::minutes(6) + std::chrono::seconds(30);
 *   meta.trackNumber = 5;
 *   meta.appMediaId = "track-12345";
 *   session->setMetadata(meta);
 * @endcode
 */
struct Metadata {
    /// Track title (required, UTF-8)
    const char* title = nullptr;
    
    /// Artist name (optional, UTF-8)
    const char* artist = nullptr;
    
    /// Album name (optional, UTF-8)
    const char* album = nullptr;
    
    /// Track duration (optional, zero = unknown)
    std::chrono::milliseconds duration{0};
    
    /// Track number in album (optional, 0 = unknown)
    uint32_t trackNumber = 0;
    
    /// App-specific media ID for track identification (optional, UTF-8)
    const char* appMediaId = nullptr;
};

// ==============================================================================
// Thumbnail Image Data
// ==============================================================================
/**
 * @brief In-memory image data for album art / thumbnails
 * 
 * Supported formats: PNG, JPEG, BMP
 * Recommended: PNG 300x300 pixels
 * Maximum: 1920x1920 pixels (Windows limitation)
 * 
 * Example:
 * @code
 *   std::vector<uint8_t> pngData = loadImageFile("cover.png");
 *   session->setThumbnail(pngData.data(), pngData.size());
 * @endcode
 */
struct ThumbnailData {
    /// Raw image data (PNG/JPEG/BMP bytes)
    const uint8_t* data = nullptr;
    
    /// Size in bytes
    size_t size = 0;
};

} // namespace windows_platform
