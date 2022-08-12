set(BINARY_NAME tkm-control)
set(BINARY_SRC 
        ${CMAKE_SOURCE_DIR}/shared/Options.cpp
        ${CMAKE_SOURCE_DIR}/shared/Helpers.cpp
        Dispatcher.cpp
        Application.cpp
        Connection.cpp
        Command.cpp
        Main.cpp
        )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${LIBS} BSWInfra)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
