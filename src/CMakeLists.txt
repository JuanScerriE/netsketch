add_executable(netsketch_server
    server/main.cpp
    server/share.cpp
    server/server.cpp
    server/conn_handler.cpp
)

target_link_libraries(netsketch_server PRIVATE
    CLI11::CLI11
    fmt::fmt
)
target_include_directories(netsketch_server PRIVATE server common)
target_compile_options(netsketch_server PRIVATE -Werror -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_client
        client/main.cpp
        client/share.cpp
        client/gui.cpp
        client/input_handler.cpp
        client/network_handler.cpp
)

 # Checks if OSX and links appropriate frameworks (only required on MacOS)
 if (APPLE)
     target_link_libraries(netsketch_client PRIVATE "-framework IOKit")
     target_link_libraries(netsketch_client PRIVATE "-framework Cocoa")
     target_link_libraries(netsketch_client PRIVATE "-framework OpenGL")
 endif()

target_link_libraries(netsketch_client PRIVATE
    CLI11::CLI11
    fmt::fmt
    raylib
)
target_include_directories(netsketch_client PRIVATE client common)
target_compile_options(netsketch_client PRIVATE -Werror -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)
