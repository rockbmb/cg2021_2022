cmake_minimum_required(VERSION 3.5)

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

link_libraries(glm ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${IL_LIBRARIES})

foreach(l ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${IL_LIBRARIES})
    message(STATUS ${l})
endforeach()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(curves src/curves.cpp src/curves.h)
link_libraries(curves)

add_executable(${PROJECT_NAME} src/main.cpp)

add_executable(generator src/generator.cpp)
add_executable(engine src/engine.cpp)

add_library(parsing src/parsing.cpp src/parsing.h)
target_link_libraries(parsing tinyxml2)

#find_package(tinyxml2 REQUIRED)
target_link_libraries(${PROJECT_NAME} tinyxml2)
target_link_libraries(engine tinyxml2 parsing ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

foreach (folder test_files_phase_1 test_files_phase_2)
    file(GLOB files "${folder}/*.sh" "${folder}/*.zsh")
    foreach (file ${files})
        get_filename_component(tname ${file} NAME)
        add_custom_target(
                ${tname}
                COMMAND ${file}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${folder}
                DEPENDS engine generator
        )
    endforeach ()
endforeach ()

add_custom_target(
        docs
        COMMAND doxygen Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
