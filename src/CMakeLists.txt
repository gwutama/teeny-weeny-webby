# Set the source files
set(SOURCES
    Configuration.cpp
    FileUtils.cpp
    GuidGenerator.cpp
    MultipartParser.cpp
    RawResponse.cpp
    Request.cpp
    Response.cpp
    Server.cpp
    ThreadPool.cpp
    main.cpp
)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set compile options to minimize binary size in Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(APPLE)
        # macOS specific flags
        target_compile_options(${PROJECT_NAME} PRIVATE
                -Os  # Optimize for size
                -fdata-sections
                -ffunction-sections
                -fno-exceptions
        )

        target_link_options(${PROJECT_NAME} PRIVATE
                -dead_strip  # Strip unused code and data
        )
    else()
        # Flags for Linux or other Unix-like systems
        target_compile_options(${PROJECT_NAME} PRIVATE
                -Os  # Optimize for size
                -fdata-sections
                -ffunction-sections
                -fno-exceptions
        )

        target_link_options(${PROJECT_NAME} PRIVATE
                -Wl,--gc-sections  # Remove unused sections
        )
    endif()
endif()
