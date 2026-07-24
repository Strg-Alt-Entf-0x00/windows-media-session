# ==============================================================================
# windows-media-session-0.8.0 - Examples
# ==============================================================================

# ==============================================================================
# Basic Player Example
# ==============================================================================
add_executable(basic_player
    basic_player.cpp
)

target_link_libraries(basic_player
    PRIVATE
        windows-media-session
)

# Set as console application (not GUI)
if(MSVC)
    set_target_properties(basic_player PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

# ==============================================================================
# Advanced Player Example (with Album Art)
# ==============================================================================
add_executable(advanced_player
    advanced_player.cpp
)

target_link_libraries(advanced_player
    PRIVATE
        windows-media-session
)

# Set as console application
if(MSVC)
    set_target_properties(advanced_player PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

# ==============================================================================
# Complete Demo (100% Features)
# ==============================================================================
add_executable(complete_demo
    complete_demo.cpp
)

target_link_libraries(complete_demo
    PRIVATE
        windows-media-session
)

# Set as console application
if(MSVC)
    set_target_properties(complete_demo PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()
