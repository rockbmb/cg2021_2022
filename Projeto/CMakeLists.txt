cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
# add_compile_definitions(USE_SYSTEM)

# Project Name
PROJECT(proj)
set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/bin")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(DevIL REQUIRED)
find_package(glm REQUIRED)
#find_package(tinyxml2 REQUIRED)

link_libraries(glm ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${IL_LIBRARIES})

foreach (l ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${IL_LIBRARIES})
    message(STATUS ${l})
endforeach ()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(curves src/curves.cpp src/curves.h)
link_libraries(curves)

add_executable(generator src/generator.cpp)
add_executable(engine src/engine.cpp)

add_library(parsing src/parsing.cpp src/parsing.h)
target_link_libraries(parsing tinyxml2)

add_library(util src/util.cpp src/util.h)

target_link_libraries(engine tinyxml2 parsing ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
add_dependencies(engine generator)

foreach (folder test_files_phase_1 test_files_phase_2 test_files_phase_3 test_files_phase_4)
    file(GLOB files "${folder}/*.sh" "${folder}/*.zsh")
    foreach (file ${files})
        get_filename_component(pname ${file} NAME_WE)
        add_custom_target(
                ${pname}
                COMMAND ${file}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${folder}
                DEPENDS engine generator
        )
    endforeach ()
endforeach ()

add_custom_target(
        docs
        COMMAND doxygen Doxyfile
        COMMAND git add docs
        COMMAND git commit -m docs
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Tests

add_custom_target(
        test_3_1
        COMMAND generator bezier teapot.patch 10 teapot.3d
        COMMAND engine test_3_1.xml
        COMMAND rm -f *.3d
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_files_phase_3
        DEPENDS engine generator
)
add_custom_target(
        test_3_2
        COMMAND generator bezier teapot.patch 10 teapot.3d
        COMMAND engine test_3_2.xml
        COMMAND rm -f *.3d
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_files_phase_3
        DEPENDS engine generator
)

file(GLOB files "test_files_phase_4/*.xml")
foreach (file ${files})
    get_filename_component(tname ${file} NAME)
    get_filename_component(pname ${file} NAME_WE)
    add_custom_target(
            ${pname}
            COMMAND engine ${tname}
            COMMAND rm -f *.3d
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_files_phase_4
            DEPENDS engine generator
    )
endforeach ()