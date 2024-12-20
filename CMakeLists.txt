cmake_minimum_required(VERSION 3.10.0)
project(project-qlc3d)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (LOG_DEBUG_HASH) # enables logging of some hash values. Helpful for detecting unintended changes until better test coverage
    add_definitions(-DLOG_DEBUG_HASH=1)
endif(LOG_DEBUG_HASH)

add_subdirectory(extern/fmt-8.1.1)
add_subdirectory(extern/SpaMtrix)
add_subdirectory(qlc3d)

