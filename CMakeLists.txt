cmake_minimum_required(VERSION 3.20)

project(hpc_spring_2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
enable_testing()

# Catch2 for unit tests
set(CATCH2_GIT_TAG "v3.11.0" CACHE STRING "Catch2 git tag/commit")
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG ${CATCH2_GIT_TAG}
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(lab1-matmul)
