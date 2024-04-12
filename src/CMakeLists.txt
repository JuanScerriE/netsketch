# add_executable(netsketch_server
# )
#
# target_link_libraries(netsketch_server PRIVATE fmt::fmt raylib)
# target_include_directories(netsketch_server PRIVATE server)
# target_compile_options(netsketch_server PRIVATE -Werror -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_client
        client/main.cpp
        client/gui.cpp
        client/command_input.cpp
)

 # Checks if OSX and links appropriate frameworks (only required on MacOS)
 if (APPLE)
     target_link_libraries(netsketch_client PRIVATE "-framework IOKit")
     target_link_libraries(netsketch_client PRIVATE "-framework Cocoa")
     target_link_libraries(netsketch_client PRIVATE "-framework OpenGL")
 endif()

target_link_libraries(netsketch_client PRIVATE fmt::fmt raylib)
target_include_directories(netsketch_client PRIVATE client)
target_compile_options(netsketch_client PRIVATE -Werror -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)
