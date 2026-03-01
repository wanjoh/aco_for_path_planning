set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_graph
    test_graph.cpp
    )

target_link_libraries(test_graph PRIVATE
    graph_lib
    Catch2::Catch2WithMain
)

add_executable(test_map
    test_map.cpp
    )

target_link_libraries(test_map PRIVATE
    map_lib
    graph_lib
    utils_lib
    Catch2::Catch2WithMain
)

add_executable(test_aco
    test_aco.cpp
)

target_link_libraries(test_aco PRIVATE
    aco_lib
    graph_lib
    map_lib
    utils_lib
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(test_graph)
catch_discover_tests(test_map)
catch_discover_tests(test_aco)