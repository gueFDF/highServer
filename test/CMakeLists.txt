# for each "test/x.cpp", generate target "x"
file(GLOB_RECURSE all_tests *.cpp)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)
foreach(v ${all_tests})
    string(REGEX MATCH "test/.*" relative_path ${v})
    # message(${relative_path})
    string(REGEX REPLACE "test/" "" target_name ${relative_path})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})

    link_libraries(utils asynlog net)
    add_executable(${target_name} ${v})
endforeach()
