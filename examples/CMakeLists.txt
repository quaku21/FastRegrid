# Create executable
add_executable(fastregrid_example example.cpp)

# Link against fastregrid INTERFACE library
target_link_libraries(fastregrid_example PRIVATE fastregrid)

# Set output directory for executable
set_target_properties(fastregrid_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable warnings
if (MSVC)
    target_compile_options(fastregrid_example PRIVATE /W4)
else()
    target_compile_options(fastregrid_example PRIVATE -Wall -Wextra -pedantic)
endif()

# Optional: Enable debug symbols
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(fastregrid_example PRIVATE -g)
endif()