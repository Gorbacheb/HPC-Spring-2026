project(lab3_raytracer LANGUAGES CXX CUDA)

include(FetchContent)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)

find_package(CUDAToolkit REQUIRED)

add_executable(lab3_raytracer src/main.cu src/bmp_utils.cpp src/scene_io.cpp src/random_scene.cpp)
target_include_directories(lab3_raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(lab3_raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common)
target_include_directories(lab3_raytracer PRIVATE ${stb_SOURCE_DIR})
target_compile_definitions(lab3_raytracer PRIVATE LAB3_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(lab3_raytracer PRIVATE CUDA::cudart CUDA::curand)
if (TARGET hpc_common)
  target_link_libraries(lab3_raytracer PRIVATE hpc_common)
endif()
set_target_properties(lab3_raytracer PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(lab3_raytracer_test src/test.cu src/bmp_utils.cpp src/scene_io.cpp src/random_scene.cpp)
target_include_directories(lab3_raytracer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(lab3_raytracer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common)
target_include_directories(lab3_raytracer_test PRIVATE ${stb_SOURCE_DIR})
target_compile_definitions(lab3_raytracer_test PRIVATE LAB3_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(lab3_raytracer_test PRIVATE CUDA::cudart CUDA::curand Catch2::Catch2WithMain)
if (TARGET hpc_common)
  target_link_libraries(lab3_raytracer_test PRIVATE hpc_common)
endif()
set_target_properties(lab3_raytracer_test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
add_test(NAME lab3_raytracer_test COMMAND lab3_raytracer_test)
