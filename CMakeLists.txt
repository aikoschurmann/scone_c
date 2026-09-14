cmake_minimum_required(VERSION 3.10)
project(scone C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable exporting compile_commands.json natively for VS Code IntelliSense!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directory definitions
set(SCHEMA_DIR "${CMAKE_SOURCE_DIR}/schemas")
set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

# Ensure generated directory exists
file(MAKE_DIRECTORY ${GEN_DIR})

# 1. Build cfg-gen tool using its own Makefile
add_custom_command(
    OUTPUT ${VENDOR_DIR}/cfgsafe/tools/out/cfg-gen
    COMMAND $(MAKE) -C ${VENDOR_DIR}/cfgsafe/tools
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building cfgsafe generator tool"
)
add_custom_target(build_cfg_gen DEPENDS ${VENDOR_DIR}/cfgsafe/tools/out/cfg-gen)

# 2. Find all schemas and generate headers
file(GLOB SCHEMAS "${SCHEMA_DIR}/*.schema")
set(GEN_HEADERS "")

foreach(SCHEMA_FILE ${SCHEMAS})
    get_filename_component(SCHEMA_NAME ${SCHEMA_FILE} NAME_WE)
    set(HEADER_OUT "${GEN_DIR}/${SCHEMA_NAME}.h")
    list(APPEND GEN_HEADERS ${HEADER_OUT})

    add_custom_command(
        OUTPUT ${HEADER_OUT}
        COMMAND ${VENDOR_DIR}/cfgsafe/tools/out/cfg-gen ${SCHEMA_FILE} -o ${HEADER_OUT}
        DEPENDS ${SCHEMA_FILE} build_cfg_gen
        COMMENT "Generating configuration header for ${SCHEMA_NAME}"
    )
endforeach()

# 3. Define the main executable
file(GLOB SRCS "${CMAKE_SOURCE_DIR}/src/*.c")
add_executable(scone ${SRCS} ${GEN_HEADERS})

# Include directories
target_include_directories(scone PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${GEN_DIR}
)
