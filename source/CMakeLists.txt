set(BINARY_NAME tkm-collector)
set(BINARY_SRC
        ${CMAKE_SOURCE_DIR}/shared/Options.cpp
        ${CMAKE_SOURCE_DIR}/shared/Helpers.cpp
        Query.cpp
        Dispatcher.cpp
        UDSServer.cpp
        ControlClient.cpp
        Application.cpp
        DeviceManager.cpp
        Connection.cpp
        MonitorDevice.cpp
        Main.cpp
        )

if(WITH_SQLITE3)
    include_directories   (${SQLITE3_INCLUDE_DIRS})
    set(BINARY_SRC ${BINARY_SRC} SQLiteDatabase.cpp)
endif()

if(WITH_POSTGRESQL)
    set(BINARY_SRC ${BINARY_SRC} PQDatabase.cpp)
endif()

add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${LIBS} BSWInfra)

install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
