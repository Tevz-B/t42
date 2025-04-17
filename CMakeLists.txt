cmake_minimum_required(VERSION 3.10)

project(t4v2)

find_package(Curses REQUIRED)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CURSES_INCLUDE_DIR})
include_directories(src)

add_executable(
    t4v2
    src/main.cpp
)


# Link libs
# if(CURSES_HAVE_NCURSESW)
    # target_link_libraries(t4 ncursesw)
# else()
    # target_link_libraries(t4 ${CURSES_LIBRARIES})
# endif()

target_link_libraries(t4v2 ${CURSES_LIBRARIES})

link_directories(/usr/lib)
include_directories(/usr/include)
