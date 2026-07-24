// ==============================================================================
// windows-media-session-0.8.0 - Thumbnail Converter
// ==============================================================================
// Description: Convert in-memory image data to WinRT streams
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#pragma once

#include <media_session/error.h>

#include <winrt/Windows.Storage.Streams.h>
#include <cstddef>
#include <cstdint>

namespace windows_platform {

// ==============================================================================
// Thumbnail Converter
// ==============================================================================
/**
 * @brief Convert in-memory image data (PNG/JPEG/BMP) to WinRT stream
 * 
 * SystemMediaTransportControls requires IRandomAccessStreamReference
 * for thumbnail images. This converter creates the necessary WinRT
 * stream from raw image bytes.
 */
class ThumbnailConverter {
public:
    /**
     * @brief Create WinRT stream from in-memory image data
     * 
     * @param data Raw image bytes (PNG/JPEG/BMP)
     * @param size Size in bytes
     * @param[out] outStream Output stream reference
     * @return ErrorInfo (Success or InvalidImageFormat or other error)
     */
    [[nodiscard]] static ErrorInfo createStreamFromMemory(
        const uint8_t* data,
        size_t size,
        winrt::Windows::Storage::Streams::RandomAccessStreamReference& outStream
    ) noexcept;

private:
    /**
     * @brief Detect image format from magic bytes
     * @return true if format is supported (PNG/JPEG/BMP)
     */
    [[nodiscard]] static bool isSupportedImageFormat(const uint8_t* data, size_t size) noexcept;
};

} // namespace windows_platform
