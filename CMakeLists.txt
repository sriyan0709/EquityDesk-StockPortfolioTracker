cmake_minimum_required(VERSION 3.15)

project(StockPortfolioTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra)
endif()

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

list(REMOVE_ITEM CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

add_library(portfolio_core ${CORE_SOURCES})

target_include_directories(portfolio_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(portfolio_tracker
    src/main.cpp
)

target_link_libraries(portfolio_tracker
    portfolio_core
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests
    portfolio_core
)

enable_testing()

add_test(
    NAME portfolio_tests
    COMMAND run_tests
)