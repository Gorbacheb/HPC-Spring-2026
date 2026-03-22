project(lab2_vectorsum LANGUAGES CXX CUDA)

find_package(CUDAToolkit REQUIRED)

add_library(lab2_vectorsum_lib
  src/vectorsum_cpu.cpp
  src/vectorsum_gpu.cu
)

target_include_directories(lab2_vectorsum_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(lab2_vectorsum_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common)
target_link_libraries(lab2_vectorsum_lib PUBLIC CUDA::cudart)
if (TARGET hpc_common)
  target_link_libraries(lab2_vectorsum_lib PUBLIC hpc_common)
endif()
set_target_properties(lab2_vectorsum_lib PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

add_executable(lab2_vectorsum src/main.cpp)
target_link_libraries(lab2_vectorsum PRIVATE lab2_vectorsum_lib)

add_executable(lab2_vectorsum_test src/test.cpp)
target_link_libraries(lab2_vectorsum_test PRIVATE lab2_vectorsum_lib Catch2::Catch2WithMain)
add_test(NAME lab2_vectorsum_test COMMAND lab2_vectorsum_test)
