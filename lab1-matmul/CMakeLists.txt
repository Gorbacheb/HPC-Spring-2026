project(lab1_matmul LANGUAGES CXX)

add_library(lab1_matmul_lib
  src/matmul_cpu.cpp
)

target_include_directories(lab1_matmul_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(lab1_matmul src/main.cpp)
target_link_libraries(lab1_matmul PRIVATE lab1_matmul_lib)
