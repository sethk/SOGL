cmake_minimum_required(VERSION 3.9)
project(SOGL)

add_compile_options(-mmacos-version-min=10.13)
add_compile_options(-Werror -Wall -Wformat -Wextra -Wno-deprecated -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers)
add_compile_options(-O0 -fno-inline)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wrap_glut.h
        COMMAND ${CMAKE_SOURCE_DIR}/gen_wrappers.rb glut ${GLUT_glut_LIBRARY}/GLUT <
                ${GLUT_INCLUDE_DIR}/glut.h > ${CMAKE_CURRENT_BINARY_DIR}/wrap_glut.h
        DEPENDS ${CMAKE_SOURCE_DIR}/gen_wrappers.rb
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(RASTER_SOURCES raster_slow.c)
#set(RASTER_SOURCES raster_fast.c)
set(DRAW_SOURCES draw_raster.c raster.c ${RASTER_SOURCES})
#set(DRAW_SOURCES draw_opengl.c)
set(SOGL_SOURCES vector.c matrix.c draw.h ${DRAW_SOURCES} window.h window_cgl.c gl.c wrap_glut.h)
function(sogl_targets name)
    add_executable(${name} ${name}.c ${ARGN} ${SOGL_SOURCES})
    add_executable(${name}_opengl ${name}.c ${ARGN})
    target_link_libraries(${name}_opengl ${GLUT_LIBRARIES})
endfunction()

sogl_targets(normal vector.c)
sogl_targets(simple)
sogl_targets(prim)
sogl_targets(cube)
sogl_targets(n64)
sogl_targets(flat)
sogl_targets(movelight)
sogl_targets(sphere)
sogl_targets(material)
sogl_targets(scene)
sogl_targets(glutplane)
sogl_targets(overlap)
sogl_targets(raster_test)
sogl_targets(checker)
add_executable(matrix_test matrix_test.c vector.c matrix.c)
add_executable(scan_test scan_test.cc ${DRAW_SOURCES} vector.c matrix.c window_null.c)
target_link_libraries(scan_test gtest_main)
add_test(NAME scan_test COMMAND scan_test)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
#add_executable(example example.cpp)
#target_link_libraries(example gtest_main)
#add_test(NAME example_test COMMAND example)
