cmake_minimum_required(VERSION 3.10)
project(ToyC_Lexer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 可执行文件
add_executable(toyc_lexer src/compiler.cpp)

# 编译选项
target_compile_options(toyc_lexer PRIVATE -Wall -Wextra)