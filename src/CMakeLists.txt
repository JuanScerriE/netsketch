option(BENCHMARK "Enable benchmarking" OFF)
option(DUMPJSON "Enable dumping canvas as JSON" OFF)
option(DUMPHASH "Enable dumping canvas hash" OFF)

add_executable(netsketch_server
        server/main.cpp
        server/conn_handler.cpp
        server/runner.cpp
        server/server.cpp
        server/share.cpp
        server/timing.cpp
        server/updater.cpp
        bench/bench.cpp
)

target_compile_definitions(netsketch_server PRIVATE
    $<$<BOOL:${BENCHMARK}>:NETSKETCH_BENCHMARK>
    $<$<BOOL:${DUMPJSON}>:NETSKETCH_DUMPJSON>
    $<$<BOOL:${DUMPHASH}>:NETSKETCH_DUMPHASH>
)

target_link_libraries(netsketch_server PRIVATE
        CLI11::CLI11
        cereal::cereal
        fmt::fmt
        spdlog::spdlog
)
target_compile_options(netsketch_server PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_client
        client/main.cpp
        client/gui.cpp
        client/input_handler.cpp
        client/input_parser.cpp
        client/reader.cpp
        client/runner.cpp
        client/share.cpp
        client/writer.cpp
        bench/bench.cpp
)

target_compile_definitions(netsketch_client PRIVATE
    $<$<BOOL:${BENCHMARK}>:NETSKETCH_BENCHMARK>
    $<$<BOOL:${DUMPJSON}>:NETSKETCH_DUMPJSON>
    $<$<BOOL:${DUMPHASH}>:NETSKETCH_DUMPHASH>
)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(netsketch_client PRIVATE "-framework IOKit")
    target_link_libraries(netsketch_client PRIVATE "-framework Cocoa")
    target_link_libraries(netsketch_client PRIVATE "-framework OpenGL")
endif ()

target_link_libraries(netsketch_client PRIVATE
        CLI11::CLI11
        cereal::cereal
        fmt::fmt
        raylib
        spdlog::spdlog
)
target_compile_options(netsketch_client PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_test_client
        test_client/main.cpp
        test_client/reader.cpp
        test_client/runner.cpp
        test_client/share.cpp
        test_client/simulate_user.cpp
        test_client/writer.cpp
        bench/bench.cpp
)

# NOTE: for the test client we want the benchmarking and
# dumping of the hash to always be active
target_compile_definitions(netsketch_test_client PRIVATE
    NETSKETCH_BENCHMARK
    NETSKETCH_DUMPHASH
    $<$<BOOL:${DUMPJSON}>:NETSKETCH_DUMPJSON>
)

target_link_libraries(netsketch_test_client PRIVATE
        CLI11::CLI11
        cereal::cereal
        fmt::fmt
        spdlog::spdlog
)
target_compile_options(netsketch_test_client PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_exporter
        exporter/main.cpp
        exporter/runner.cpp
)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(netsketch_exporter PRIVATE "-framework IOKit")
    target_link_libraries(netsketch_exporter PRIVATE "-framework Cocoa")
    target_link_libraries(netsketch_exporter PRIVATE "-framework OpenGL")
endif ()

target_link_libraries(netsketch_exporter PRIVATE
        CLI11::CLI11
        cereal::cereal
        fmt::fmt
        raylib
        spdlog::spdlog
)
target_compile_options(netsketch_exporter PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

