# Windows Media Session

[![Version](https://img.shields.io/badge/version-0.8.0-blue.svg)](https://github.com/Strg-Alt-Entf-0x00/windows-media-session)
[![License](https://img.shields.io/badge/license-Public%20Domain-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-orange.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

C++20 library for Windows Media Session integration with system-wide media transport controls and now playing information.

## Features

- 🎵 **System Media Controls** - Integrates with Windows media transport controls
- ⌨️ **Media Keys** - Responds to hardware media keys (Play, Pause, Next, Previous)
- 📱 **SMTC Integration** - SystemMediaTransportControls for Windows 10/11
- 🖼️ **Thumbnail Support** - Display album art and track information
- 📊 **Playback Timeline** - Track position and duration display
- 🔄 **State Management** - Automatic playback state synchronization
- 🎯 **Modern C++20** - RAII, coroutines, and type-safe API

## Requirements

- **CMake**: 3.20 or higher
- **Compiler**: C++20 support required
  - MSVC 2019 16.11+ (Visual Studio 2019)
  - MSVC 2022 (recommended)
- **Platform**: Windows 10 1607+ or Windows 11
- **Windows SDK**: 10.0.17134.0 or higher

## Quick Start

### Building

```bash
# Using the provided build script
build.bat

# Or manually
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Basic Usage

```cpp
#include <media_session/media_session.h>

int main() {
    // Create media session
    media_session::MediaSession session;
    
    // Set metadata
    session.set_title(L"Song Title");
    session.set_artist(L"Artist Name");
    session.set_album(L"Album Name");
    
    // Set playback state
    session.set_playback_state(media_session::PlaybackState::Playing);
    
    // Set timeline
    session.set_timeline(180.0, 45.5); // duration, position in seconds
    
    // Register callbacks
    session.on_play_requested([]() {
        // Handle play
    });
    
    session.on_pause_requested([]() {
        // Handle pause
    });
    
    session.on_next_requested([]() {
        // Handle next track
    });
    
    session.on_previous_requested([]() {
        // Handle previous track
    });
    
    return 0;
}
```

### Complete Music Player Integration

```cpp
#include <media_session/media_session.h>

class MusicPlayer {
public:
    MusicPlayer() {
        setup_media_session();
    }
    
    void play_track(const Track& track) {
        current_track_ = track;
        
        // Update media session
        session_.set_title(track.title);
        session_.set_artist(track.artist);
        session_.set_album(track.album);
        session_.set_thumbnail(track.album_art_path);
        session_.set_timeline(track.duration, 0.0);
        session_.set_playback_state(media_session::PlaybackState::Playing);
        
        // Start playback
        start_playback();
    }
    
    void update_position(double position) {
        session_.set_timeline(current_track_.duration, position);
    }
    
private:
    media_session::MediaSession session_;
    Track current_track_;
    
    void setup_media_session() {
        session_.on_play_requested([this]() {
            resume();
        });
        
        session_.on_pause_requested([this]() {
            pause();
        });
        
        session_.on_next_requested([this]() {
            next_track();
        });
        
        session_.on_previous_requested([this]() {
            previous_track();
        });
        
        session_.on_seek_requested([this](double position) {
            seek_to(position);
        });
    }
};
```

### Album Art / Thumbnail

```cpp
// Set thumbnail from file
session.set_thumbnail(L"C:\\Music\\album_art.jpg");

// Or from raw bitmap data
std::vector<uint8_t> bitmap_data = load_album_art();
session.set_thumbnail_from_memory(bitmap_data.data(), 
                                   bitmap_data.size(),
                                   300, 300); // width, height
```

## API Reference

### MediaSession Class

#### Metadata
```cpp
void set_title(const std::wstring& title);
void set_artist(const std::wstring& artist);
void set_album(const std::wstring& album);
void set_thumbnail(const std::wstring& file_path);
void set_thumbnail_from_memory(const uint8_t* data, size_t size, 
                               int width, int height);
```

#### Playback Control
```cpp
void set_playback_state(PlaybackState state);
void set_timeline(double duration, double position);
void set_shuffle_enabled(bool enabled);
void set_repeat_mode(RepeatMode mode);
```

Playback states:
- `PlaybackState::Stopped`
- `PlaybackState::Playing`
- `PlaybackState::Paused`

Repeat modes:
- `RepeatMode::None`
- `RepeatMode::Track`
- `RepeatMode::List`

#### Event Callbacks
```cpp
void on_play_requested(std::function<void()> callback);
void on_pause_requested(std::function<void()> callback);
void on_stop_requested(std::function<void()> callback);
void on_next_requested(std::function<void()> callback);
void on_previous_requested(std::function<void()> callback);
void on_seek_requested(std::function<void(double)> callback);
void on_shuffle_requested(std::function<void(bool)> callback);
void on_repeat_requested(std::function<void(RepeatMode)> callback);
```

#### Button Visibility
```cpp
void enable_button(MediaButton button);
void disable_button(MediaButton button);
```

Media buttons:
- `MediaButton::Play`
- `MediaButton::Pause`
- `MediaButton::Stop`
- `MediaButton::Next`
- `MediaButton::Previous`

## CMake Integration

```cmake
# In your CMakeLists.txt
add_subdirectory(third-party-cpp/windows-media-session-0.8.0)

add_executable(your_media_app main.cpp)
target_link_libraries(your_media_app PRIVATE windows-media-session)
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_EXAMPLES` | `OFF` | Build example programs |
| `BUILD_TESTING` | `OFF` | Build unit tests |

## Examples

The library includes several example programs:

- `basic_player.cpp` - Simple media session setup
- `advanced_player.cpp` - Full-featured implementation
- `complete_demo.cpp` - Comprehensive feature demonstration

Build examples:
```bash
cd build
cmake -DBUILD_EXAMPLES=ON ..
cmake --build . --config Release
```

## Integration with Windows

### Windows 10 Media Controls

Your application will appear in:
- Windows 10/11 Media Controls (Volume overlay)
- Action Center media widget
- Lock screen media controls
- Bluetooth media controls

### Hardware Media Keys

Supports physical keyboard media keys:
- Play/Pause
- Next Track
- Previous Track
- Stop

## Project Structure

```
windows-media-session-0.8.0/
├── include/
│   └── media_session/
│       ├── media_session.h
│       └── types.h
├── src/
│   ├── media_session.cpp
│   ├── media_session_impl.cpp
│   ├── media_session_impl.h
│   ├── thumbnail_converter.cpp
│   └── thumbnail_converter.h
├── examples/
│   ├── basic_player.cpp
│   ├── advanced_player.cpp
│   └── complete_demo.cpp
├── tests/
├── build.bat
├── CMakeLists.txt
└── README.md
```

## Best Practices

### Update Frequency
- Update position every 1-2 seconds during playback
- Don't update position more than once per second
- Only update state when it actually changes

### Thumbnail Images
- Recommended size: 300x300 pixels
- Supported formats: JPG, PNG, BMP
- Keep file size under 1MB for best performance

### Error Handling
```cpp
try {
    session.set_thumbnail(L"album_art.jpg");
} catch (const media_session::Exception& ex) {
    // Handle error
    log_error(ex.what());
}
```

## Troubleshooting

### Controls Not Appearing
- Ensure Windows 10 1607 or higher
- Check that media keys are not disabled in system settings
- Verify your app has focus at least once

### Album Art Not Showing
- Check file path is absolute and exists
- Verify image format is supported
- Ensure image dimensions are reasonable (< 4000x4000)

## License

MIT License - See LICENSE file for details

