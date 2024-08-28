# Set the source files
set(SOURCES
    Configuration.cpp
        FileUtils.cpp
    GuidGenerator.cpp
    MultipartParser.cpp
    Request.cpp
    Response.cpp
    Server.cpp
    ThreadPool.cpp
    main.cpp
)

# Add the executable
add_executable(tww ${SOURCES})

