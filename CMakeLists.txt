cmake_minimum_required(VERSION 3.14)
project(ltlmc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# add_compile_definitions(DEBUG)
# add_compile_definitions(CYCLE)
add_executable(ltlmc
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/nba.cpp
    src/ts.cpp
    src/prod.cpp
)

target_include_directories(ltlmc PRIVATE include)
