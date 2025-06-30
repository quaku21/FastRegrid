# Define header files
set(HEADERS
    config.h
    types.h
    utils.h
    io.h
    spatial_index.h
    interpolation.h
    regridder.h
    logger.h
    filesystem.h
)

# Create header-only library
add_library(fastregrid INTERFACE)

# Set include directories for INTERFACE library
target_include_directories(fastregrid INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/fastregrid>
)

# Optional: Install library and headers
install(TARGETS fastregrid
    EXPORT FastregridTargets
    DESTINATION lib/cmake/Fastregrid
)
install(FILES ${HEADERS} DESTINATION include/fastregrid)
install(EXPORT FastregridTargets
    FILE FastregridConfig.cmake
    DESTINATION lib/cmake/Fastregrid
)