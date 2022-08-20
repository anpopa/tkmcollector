cmake_minimum_required(VERSION 3.7.2)
project(TaskMonitorCollector
        LANGUAGES CXX
        VERSION 1.5.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/bswinfra/cmake")

# options
option(WITH_TESTS "Build test suite" N)
option(WITH_SYSLOG "Build Logger with syslog" Y)
option(WITH_TIDY "Build with clang-tidy" N)
option(WITH_SQLITE3 "Build with SQLite3 database" Y)
option(WITH_POSTGRESQL "Build with PostgreSQL database" Y)
option(WITH_SYSTEMD "Build with systemd watchdog support" Y)

# Build time configuration setup
if(EXISTS .git)
    execute_process(
        COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(PROJECT_VERSION_TWEAK ${GIT_SHA1})
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

# cmake modules
include(GNUInstallDirs)
include(Packing)
include(BSWInfra)

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

set(TKM_COLLECTOR_CONFIG_FILE "/etc/tkmcollector.conf" CACHE PATH "Default config file path")

configure_file(
    ${CMAKE_SOURCE_DIR}/shared/Defaults.h.in
    ${CMAKE_BINARY_DIR}/shared/Defaults.h)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkmcollector.conf.in
    ${CMAKE_BINARY_DIR}/config/tkmcollector.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkmcollector.service.in
    ${CMAKE_BINARY_DIR}/config/tkmcollector.service)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/tkmcollector.in
    ${CMAKE_BINARY_DIR}/config/tkmcollector)

# Dependencies
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(tkm REQUIRED)

set(TKMCOLLECTOR_SRC
    shared/Options.cpp
    shared/Helpers.cpp
    source/Query.cpp
    source/Dispatcher.cpp
    source/UDSServer.cpp
    source/ControlClient.cpp
    source/Application.cpp
    source/DeviceManager.cpp
    source/Connection.cpp
    source/MonitorDevice.cpp
    source/Main.cpp
)

set(TKMCONTROL_SRC
    shared/Options.cpp
    shared/Helpers.cpp
    control/Dispatcher.cpp
    control/Application.cpp
    control/Connection.cpp
    control/Command.cpp
    control/Main.cpp
)

if(WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")
endif()

if(WITH_SYSLOG)
    add_compile_options("-DWITH_SYSLOG")
endif()

if(WITH_SYSTEMD)
    pkg_check_modules(LIBSYSTEMD libsystemd REQUIRED)
    include_directories(${LIBSYSTEMD_INCLUDE_DIRS})
    add_compile_options("-DWITH_SYSTEMD")
endif()

if(WITH_SQLITE3)
    find_package(SQLite3 REQUIRED)
    add_compile_options("-DWITH_SQLITE3")
    set(TKMCOLLECTOR_SRC ${TKMCOLLECTOR_SRC} source/SQLiteDatabase.cpp)
endif()

if(WITH_POSTGRESQL)
    pkg_check_modules(LIBPQXX libpqxx REQUIRED)
    include_directories(${LIBPQXX_INCLUDE_DIRS})
    add_compile_options("-DWITH_POSTGRESQL")
    set(TKMCOLLECTOR_SRC ${TKMCOLLECTOR_SRC} source/PQDatabase.cpp)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
    link_directories(/usr/local/lib)
endif()

# Header files
include_directories(${CMAKE_SOURCE_DIR}/shared)
include_directories(${CMAKE_BINARY_DIR}/shared)

# Build control project
#add_subdirectory(control)

# binary
add_executable(tkmcollector
    ${TKMCOLLECTOR_SRC}
)

target_link_libraries(tkmcollector
    PRIVATE
        BSWInfra
        pthread
        tkm::tkm
        sqlite3
        ${PROTOBUF_LIBRARY}
        ${LIBSYSTEMD_LIBRARIES}
        ${LIBPQXX_LIBRARIES}
)

add_executable(tkmcontrol
    ${TKMCONTROL_SRC}
)

target_link_libraries(tkmcontrol
    PRIVATE
        BSWInfra
        pthread
        tkm::tkm
        ${PROTOBUF_LIBRARY}
)

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_BINARY_DIR}/source
)

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

# install
install(TARGETS tkmcollector RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS tkmcontrol RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
        ${CMAKE_BINARY_DIR}/config/tkmcollector.conf
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
    install(FILES
        ${CMAKE_BINARY_DIR}/config/tkmcollector.conf
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/etc)
    install(FILES
        ${CMAKE_BINARY_DIR}/config/tkmcollector
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/rc.d)
else()
    install(FILES
        ${CMAKE_BINARY_DIR}/config/tkmcollector.conf
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR})
endif()

if(WITH_SYSTEMD)
    install(FILES
        ${CMAKE_BINARY_DIR}/config/tkmcollector.service
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR}/systemd/system)
endif()

# Status reporting
message (STATUS "SYSTEM_TYPE: "          ${CMAKE_SYSTEM_NAME})
message (STATUS "CMAKE_BUILD_TYPE: "     ${CMAKE_BUILD_TYPE})
message (STATUS "WITH_SYSTEMD: "         ${WITH_SYSTEMD})
message (STATUS "WITH_TESTS: "           ${WITH_TESTS})
message (STATUS "WITH_SYSLOG: "          ${WITH_SYSLOG})
message (STATUS "WITH_TIDY: "            ${WITH_TIDY})
message (STATUS "WITH_SQLITE3: "         ${WITH_SQLITE3})
message (STATUS "WITH_POSTGRESQL: "      ${WITH_POSTGRESQL})

