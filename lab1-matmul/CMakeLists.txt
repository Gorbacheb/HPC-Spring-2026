project(lab1_matmul LANGUAGES CXX CUDA)

find_package(CUDAToolkit REQUIRED)

add_library(lab1_matmul_lib
  src/matmul_cpu.cpp
  src/matmul_gpu.cu
)

target_include_directories(lab1_matmul_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(lab1_matmul_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common)
target_link_libraries(lab1_matmul_lib PUBLIC CUDA::cudart)
if (TARGET hpc_common)
  target_link_libraries(lab1_matmul_lib PUBLIC hpc_common)
endif()
set_target_properties(lab1_matmul_lib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(lab1_matmul src/main.cpp)
target_link_libraries(lab1_matmul PRIVATE lab1_matmul_lib)

add_executable(lab1_matmul_test src/test.cpp)
target_link_libraries(lab1_matmul_test PRIVATE lab1_matmul_lib Catch2::Catch2WithMain)
add_test(NAME lab1_matmul_test COMMAND lab1_matmul_test)
