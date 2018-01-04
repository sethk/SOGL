cmake_minimum_required(VERSION 3.9)
project(SOGL)

add_compile_options(-Wall -Wformat -Wextra -Wno-deprecated -Wno-unused-parameter)
add_compile_options(-O0 -fno-inline)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wrap_glut.h
        COMMAND ${CMAKE_SOURCE_DIR}/gen_wrappers.rb glut /System/Library/Frameworks/GLUT.framework/GLUT <
                /System/Library/Frameworks/GLUT.framework/Headers/glut.h > ${CMAKE_CURRENT_BINARY_DIR}/wrap_glut.h
        DEPENDS ${CMAKE_SOURCE_DIR}/gen_wrappers.rb
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOGL_SOURCES gl.c wrap_glut.h)
function(sogl_targets name)
    add_executable(${name} ${name}.c ${SOGL_SOURCES})
    add_executable(opengl_${name} ${name}.c)
    target_link_libraries(opengl_${name} ${GLUT_LIBRARIES})
endfunction()

sogl_targets(prim)
sogl_targets(cube)
sogl_targets(n64)
sogl_targets(flat)
sogl_targets(movelight)
sogl_targets(sphere)
sogl_targets(scene)
sogl_targets(glutplane)
