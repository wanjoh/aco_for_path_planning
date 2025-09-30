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

include(CTest)
include(Catch)
catch_discover_tests(test_graph)
