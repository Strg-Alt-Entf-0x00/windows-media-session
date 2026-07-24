// ==============================================================================
// windows-media-session-0.8.0 - Thumbnail Converter Implementation
// ==============================================================================
// Description: Convert in-memory images to WinRT streams
// License:     MIT
// Created:     2026-06-22
// ==============================================================================

#include "thumbnail_converter.h"

#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.h>

namespace windows_platform {

// ==============================================================================
// Image Format Detection
// ==============================================================================
bool ThumbnailConverter::isSupportedImageFormat(const uint8_t* data, size_t size) noexcept {
    if (!data || size < 4) {
        return false;
    }
    
    // PNG magic bytes: 89 50 4E 47 (0x89 'P' 'N' 'G')
    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
        return true;
    }
    
    // JPEG magic bytes: FF D8 FF
    if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
        return true;
    }
    
    // BMP magic bytes: 42 4D ('B' 'M')
    if (data[0] == 0x42 && data[1] == 0x4D) {
        return true;
    }
    
    return false;
}

// ==============================================================================
// Stream Creation
// ==============================================================================
ErrorInfo ThumbnailConverter::createStreamFromMemory(
    const uint8_t* data,
    size_t size,
    winrt::Windows::Storage::Streams::RandomAccessStreamReference& outStream
) noexcept {
    // Validate input
    if (!data || size == 0) {
        return {ErrorCode::InvalidParameter, "Invalid image data"};
    }
    
    // Check image format
    if (!isSupportedImageFormat(data, size)) {
        return {ErrorCode::InvalidImageFormat, "Unsupported image format (expected PNG/JPEG/BMP)"};
    }
    
    try {
        // Create in-memory stream
        auto stream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
        
        // Create DataWriter for writing bytes
        auto writer = winrt::Windows::Storage::Streams::DataWriter(stream);
        
        // Write image data to stream
        writer.WriteBytes(winrt::array_view<const uint8_t>(data, data + size));
        
        // Flush writer (commit data to stream)
        writer.StoreAsync().get();  // Synchronous wait
        writer.DetachStream();      // Release stream ownership
        
        // Seek to beginning (important for reading)
        stream.Seek(0);
        
        // Create RandomAccessStreamReference from stream
        outStream = winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromStream(stream);
        
        return kSuccess;
    }
    catch (const winrt::hresult_error&) {
        return {ErrorCode::WinRTFailed, "Failed to create WinRT stream"};
    }
    catch (const std::bad_alloc&) {
        return {ErrorCode::OutOfMemory, "Out of memory"};
    }
    catch (...) {
        return {ErrorCode::UnknownError, "Unknown error during stream creation"};
    }
}

} // namespace windows_platform
