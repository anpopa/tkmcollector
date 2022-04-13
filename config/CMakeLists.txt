install(FILES ${CMAKE_BINARY_DIR}/config/tkm-collector.conf DESTINATION /etc)
install(FILES ${CMAKE_BINARY_DIR}/config/tkm-collector.service DESTINATION /etc/systemd/system)