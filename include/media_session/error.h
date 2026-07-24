// ==============================================================================
// windows-media-session-0.8.0 - Error Handling
// ==============================================================================
// Description: ErrorInfo-based error handling (no exceptions in public API)
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#pragma once

namespace windows_platform {

// ==============================================================================
// Error Codes
// ==============================================================================
/**
 * @brief Error codes for MediaSession operations
 * 
 * All public API functions return ErrorInfo instead of throwing exceptions
 * for predictable, exception-free error handling.
 */
enum class ErrorCode {
    /// Operation succeeded
    Success = 0,
    
    /// MediaSession not initialized (factory returned nullptr)
    NotInitialized = 1,
    
    /// Invalid parameter provided (nullptr, empty string, etc.)
    InvalidParameter = 2,
    
    /// WinRT API call failed (SystemMediaTransportControls error)
    WinRTFailed = 3,
    
    /// Unsupported OS version (requires Windows 10 Build 14393+)
    UnsupportedOS = 4,
    
    /// Out of memory
    OutOfMemory = 5,
    
    /// Invalid image format (not PNG/JPEG/BMP)
    InvalidImageFormat = 6,
    
    /// Unknown error
    UnknownError = 99
};

// ==============================================================================
// Error Info Structure
// ==============================================================================
/**
 * @brief Error information returned by all public API functions
 * 
 * Example usage:
 * @code
 *   auto error = session->setPlaybackState(PlaybackState::Playing);
 *   if (!error.isSuccess()) {
 *       fprintf(stderr, "Error: %s (code: %d)\n", error.message, (int)error.code);
 *       return;
 *   }
 * @endcode
 */
struct ErrorInfo {
    /// Error code (Success = 0)
    ErrorCode code;
    
    /// Human-readable error message (UTF-8)
    const char* message;
    
    /**
     * @brief Check if operation succeeded
     * @return true if code == ErrorCode::Success
     */
    [[nodiscard]] constexpr bool isSuccess() const noexcept {
        return code == ErrorCode::Success;
    }
    
    /**
     * @brief Check if operation failed
     * @return true if code != ErrorCode::Success
     */
    [[nodiscard]] constexpr bool isError() const noexcept {
        return code != ErrorCode::Success;
    }
    
    /**
     * @brief Explicit boolean conversion (for if-checks)
     * @return true if operation succeeded
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return isSuccess();
    }
};

// ==============================================================================
// Success Constant
// ==============================================================================
/// Convenience constant for successful operations
inline constexpr ErrorInfo kSuccess{ErrorCode::Success, "Success"};

} // namespace windows_platform
