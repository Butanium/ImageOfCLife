cmake_minimum_required(VERSION 3.9)
project(ImageOfCLife)
set(CMAKE_C_STANDARD 99)

set(SDL2_PATH "C:\\Users\\Clement\\Documents\\coding\\libraries\\SDL2-devel-2.0.14-mingw\\SDL2-2.0.14\\i686-w64-mingw32")
find_package(SDL2 REQUIRED)
include_directories( ${SDL2_INCLUDE_DIR} )

add_executable(ImageOfCLife main.c)

target_link_libraries(ImageOfCLife ${SDL2_LIBRARY})