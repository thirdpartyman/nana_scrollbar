cmake_minimum_required(VERSION 3.5)

project(nana_scrollbar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nana_scrollbar example/main.cpp)


find_package(PkgConfig REQUIRED)

pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
pkg_check_modules(XCURSOR REQUIRED IMPORTED_TARGET xcursor)


find_package(Threads REQUIRED)

target_link_libraries(nana_scrollbar PUBLIC
        ${NANA_LIBRARY}
        PkgConfig::X11
        PkgConfig::XCURSOR
        Threads::Threads)

target_include_directories(nana_scrollbar PUBLIC include)
