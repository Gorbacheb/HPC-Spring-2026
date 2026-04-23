cmake_minimum_required(VERSION 3.20)

set(HPC_CUDA_PLATFORM "auto" CACHE STRING "CUDA platform: auto|linux|windows")
set_property(CACHE HPC_CUDA_PLATFORM PROPERTY STRINGS auto linux windows)
set(HPC_CUDA_NVCC_PATH "" CACHE FILEPATH "Override path to nvcc")

# Help CMake find nvcc in WSL/IDE environments.
if (DEFINED CMAKE_CUDA_COMPILER AND CMAKE_CUDA_COMPILER STREQUAL "")
  unset(CMAKE_CUDA_COMPILER CACHE)
endif()
if (HPC_CUDA_NVCC_PATH)
  set(CMAKE_CUDA_COMPILER "${HPC_CUDA_NVCC_PATH}" CACHE FILEPATH "CUDA compiler" FORCE)
endif()
if (NOT DEFINED CMAKE_CUDA_COMPILER)
  set(_cuda_paths)
  if (HPC_CUDA_PLATFORM STREQUAL "windows")
    list(APPEND _cuda_paths
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.1/bin/nvcc.exe"
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.0/bin/nvcc.exe"
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.4/bin/nvcc.exe"
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/bin/nvcc.exe"
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/bin/nvcc.exe"
      "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/bin/nvcc.exe"
    )
  elseif (HPC_CUDA_PLATFORM STREQUAL "linux")
    list(APPEND _cuda_paths
      "/usr/local/cuda/bin/nvcc"
      "/opt/cuda/bin/nvcc"
    )
  else()
    if (WIN32)
      list(APPEND _cuda_paths
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.1/bin/nvcc.exe"
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.0/bin/nvcc.exe"
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.4/bin/nvcc.exe"
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/bin/nvcc.exe"
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/bin/nvcc.exe"
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/bin/nvcc.exe"
      )
    else()
      list(APPEND _cuda_paths
        "/usr/local/cuda/bin/nvcc"
        "/opt/cuda/bin/nvcc"
      )
    endif()
  endif()

  foreach(_cuda_path IN LISTS _cuda_paths)
    if (EXISTS "${_cuda_path}")
      set(CMAKE_CUDA_COMPILER "${_cuda_path}" CACHE FILEPATH "CUDA compiler" FORCE)
      break()
    endif()
  endforeach()
endif()

# CMake 3.27+ errors out if CMAKE_CUDA_ARCHITECTURES is set to empty.
if (DEFINED CMAKE_CUDA_ARCHITECTURES AND CMAKE_CUDA_ARCHITECTURES STREQUAL "")
  unset(CMAKE_CUDA_ARCHITECTURES CACHE)
endif()
if (NOT DEFINED CMAKE_CUDA_ARCHITECTURES OR CMAKE_CUDA_ARCHITECTURES STREQUAL "")
  set(CMAKE_CUDA_ARCHITECTURES native)
endif()

project(hpc_spring_2026 LANGUAGES CXX CUDA)

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

# csv2 for CSV writing
set(CSV2_GIT_TAG "master" CACHE STRING "csv2 git tag/commit")
FetchContent_Declare(
  csv2
  GIT_REPOSITORY https://github.com/p-ranav/csv2.git
  GIT_TAG ${CSV2_GIT_TAG}
  GIT_SHALLOW TRUE
)
FetchContent_Populate(csv2)
add_library(csv2 INTERFACE)
add_library(csv2::csv2 ALIAS csv2)
target_include_directories(csv2 INTERFACE ${csv2_SOURCE_DIR}/include)

add_library(hpc_common INTERFACE)
target_include_directories(hpc_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(hpc_common INTERFACE csv2::csv2)
target_sources(hpc_common INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/common/cuda_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/common/csv_writer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/common/format_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/common/math_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/common/random_utils.h
)

add_subdirectory(lab1-matmul)
add_subdirectory(lab2-vectorsum)
add_subdirectory(lab3-raytracer)
