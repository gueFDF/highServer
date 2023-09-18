# for each "example/x.cpp", generate target "x"
file(GLOB_RECURSE all_examples *.cpp)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin//example)
foreach(v ${all_examples})
    string(REGEX MATCH "example/.*" relative_path ${v})
    # message(${relative_path})
    string(REGEX REPLACE "example/" "" target_name ${relative_path})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})
    link_libraries(utils asynlog)
    add_executable(${target_name} ${v})
endforeach()

