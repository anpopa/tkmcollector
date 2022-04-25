# CMake general setup
cmake_minimum_required(VERSION 3.7.2)
project(TaskMonitorCollector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# BaseSoftwareInfrastructure
set(BSWINFRA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bswinfra)
set(INTERFACES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/interfaces)
set(CMAKE_MODULE_PATH "${BSWINFRA_DIR}/cmake" "${INTERFACES_DIR}/cmake")

message (STATUS "Module paths: ${CMAKE_MODULE_PATH}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

# Add support for coverage analysis
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    set(COVERAGE_EXCLUDES
        "*/bswinfra/*"
        "*/build/*"
        "*/tests/*"
        )
    set(COVERAGE_BASE_DIR ${CMAKE_SOURCE_DIR}/source)
    set(COVERAGE_BRANCH_COVERAGE ON)
    set(COVERAGE_THRESHOLD_LINE 90)
    set(COVERAGE_THRESHOLD_FUNCTION 90)

    include(CoverageTarget)
endif()

# Build time configuration setup
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_SHA1 "")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

set(TKM_COLLECTOR_CONFIG_FILE "/etc/tkm-collector.conf" CACHE PATH "Default config file path")

configure_file(
    ${CMAKE_SOURCE_DIR}/shared/Defaults.h.in
    ${CMAKE_BINARY_DIR}/shared/Defaults.h)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkm-collector.conf.in
    ${CMAKE_BINARY_DIR}/config/tkm-collector.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkm-collector.service.in
    ${CMAKE_BINARY_DIR}/config/tkm-collector.service)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkmcollector.in
    ${CMAKE_BINARY_DIR}/config/tkmcollector)



option(WITH_TESTS "Build test suite" N)
option(WITH_SYSLOG "Build Logger with syslog" Y)
option(WITH_TIDY "Build with clang-tidy" N)
option(WITH_SQLITE3 "Build with SQLite3 database" Y)
option(WITH_POSTGRESQL "Build with PostgreSQL database" Y)
option(WITH_SYSTEMD "Build with systemd watchdog support" Y)

if(WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")
endif()

if(WITH_SYSLOG)
    add_compile_options("-DWITH_SYSLOG")
endif()

if(WITH_SQLITE3)
    add_compile_options("-DWITH_SQLITE3")
endif()

if(WITH_POSTGRESQL)
    add_compile_options("-DWITH_POSTGRESQL")
endif()

# Enable collector interfaces
set(WITH_COLLECTOR_CONTROL ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
    link_directories(/usr/local/lib)
endif()

include(BSWInfra)
include(TaskMonitorInterfaces)

# Header files
include_directories(${CMAKE_SOURCE_DIR}/shared)
include_directories(${CMAKE_BINARY_DIR}/shared)

# Dependencies
find_package    (PkgConfig REQUIRED)
find_package    (Threads REQUIRED)

if(WITH_SQLITE3)
    find_package      (SQLite3 REQUIRED)
    include_directories(${SQLITE3_INCLUDE_DIR})
endif()

if(WITH_POSTGRESQL)
    pkg_check_modules     (LIBPQXX libpqxx REQUIRED)
    include_directories   (${LIBPQXX_INCLUDE_DIRS})
    set                   (LIBS ${LIBS} ${LIBPQXX_LIBRARIES})
endif()

if(WITH_SYSTEMD)
    pkg_check_modules     (LIBSYSTEMD libsystemd REQUIRED)
    include_directories   (${LIBSYSTEMD_INCLUDE_DIRS})
    set                   (LIBS ${LIBS} ${LIBSYSTEMD_LIBRARIES})
endif()

# Build
add_subdirectory(config)
add_subdirectory(source)
add_subdirectory(control)
if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Build flags
add_compile_options (
    -Wall
    -Wextra
    -Wno-unused-function
    -Wformat
    -Wno-variadic-macros
    -Wno-strict-aliasing
    -D_FORTIFY_SOURCE=2
    -fstack-protector-strong
    -fwrapv
    -Wformat-signedness
    -Wmissing-include-dirs
    -Wimplicit-fallthrough=5
    -Wunused-parameter
    -Wuninitialized
    -Walloca
    -Wduplicated-branches
    -Wduplicated-cond
    -Wfloat-equal
    -Wshadow
    -Wcast-qual
    -Wconversion
    -Wsign-conversion
    -Wlogical-op
    -Werror
    -Wformat-security
    -Walloc-zero
    -Wcast-align
    -Wredundant-decls
    )

# Status reporting
message (STATUS "SYSTEM_TYPE: "          ${CMAKE_SYSTEM_NAME})
message (STATUS "CMAKE_BUILD_TYPE: "     ${CMAKE_BUILD_TYPE})
message (STATUS "WITH_SYSTEMD: "         ${WITH_SYSTEMD})
message (STATUS "WITH_TESTS: "           ${WITH_TESTS})
message (STATUS "WITH_SYSLOG: "          ${WITH_SYSLOG})
message (STATUS "WITH_TIDY: "            ${WITH_TIDY})
message (STATUS "WITH_SQLITE3: "         ${WITH_SQLITE3})
message (STATUS "WITH_POSTGRESQL: "      ${WITH_POSTGRESQL})

