set(BINARY_NAME tkm-control)
set(BINARY_SRC 
        ${CMAKE_SOURCE_DIR}/shared/Options.cpp
        ${CMAKE_SOURCE_DIR}/shared/Helpers.cpp
        ${CMAKE_SOURCE_DIR}/shared/EnvelopeReader.cpp
        ${CMAKE_SOURCE_DIR}/shared/EnvelopeWriter.cpp
        Dispatcher.cpp
        Application.cpp
        Connection.cpp
        Command.cpp
        Main.cpp
        )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${OPENSSL_CRYPTO_LIBRARY} BSWInfra TaskMonitorInterfaces)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
