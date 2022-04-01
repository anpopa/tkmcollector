include(GNUInstallDirs)

# GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/source)

set(MODULES_SRC
        ${CMAKE_SOURCE_DIR}/shared/Options.cpp
        ${CMAKE_SOURCE_DIR}/shared/Helpers.cpp
        ${CMAKE_SOURCE_DIR}/shared/Query.cpp
        ${CMAKE_SOURCE_DIR}/shared/EnvelopeReader.cpp
        ${CMAKE_SOURCE_DIR}/shared/EnvelopeWriter.cpp
        ${CMAKE_SOURCE_DIR}/source/ViewerClient.cpp
        ${CMAKE_SOURCE_DIR}/source/ViewerSession.cpp
        ${CMAKE_SOURCE_DIR}/source/IP4Server.cpp
        ${CMAKE_SOURCE_DIR}/source/Dispatcher.cpp
        ${CMAKE_SOURCE_DIR}/source/SessionManager.cpp
        ${CMAKE_SOURCE_DIR}/source/UDSServer.cpp
        ${CMAKE_SOURCE_DIR}/source/AdminClient.cpp
        ${CMAKE_SOURCE_DIR}/source/Application.cpp
        )

if(WITH_SQLITE3)
  set(MODULES_SRC ${MODULES_SRC} ${CMAKE_SOURCE_DIR}/source/SQLiteDatabase.cpp)
endif()

configure_file(
        ${CMAKE_SOURCE_DIR}/tests/assets/cds-server.conf
        ${CMAKE_BINARY_DIR}/tests/assets/cds-server.conf)

# Testcases
add_executable(gtest_application ${MODULES_SRC} gtest_application.cpp)
target_link_libraries(gtest_application
        ${LIBS}
        ${GMOCK_LIBRARIES}
        ${GTEST_LIBRARIES}
        ${SQLite3_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARY}
        BSWInfra
        CDSInterfaces)
add_test(NAME gtest_application WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND gtest_application)

if (WITH_DEBUG_DEPLOY)
    install(TARGETS gtest_application RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif ()
