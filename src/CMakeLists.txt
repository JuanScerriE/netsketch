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
)

target_link_libraries(netsketch_client PRIVATE fmt::fmt raylib)
target_include_directories(netsketch_client PRIVATE client)
target_compile_options(netsketch_client PRIVATE -Werror -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)
