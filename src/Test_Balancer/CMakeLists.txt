set(EXECUTABLE_FILES Test_Balancer)

add_library(CUSTOM_BALANCER_UTILS SHARED src/CustomBalancer.cpp)
target_link_libraries(CUSTOM_BALANCER_UTILS PUBLIC ${LIBS})

foreach(executable_file ${EXECUTABLE_FILES})

    set(executable_sources src/${executable_file}.cpp)
    set(executable_headers src/${executable_file}.h)

    add_executable(${executable_file} ${executable_sources} ${executable_headers})
    target_link_libraries(${executable_file} PUBLIC ${LIBS} CUSTOM_BALANCER_UTILS)
endforeach ()