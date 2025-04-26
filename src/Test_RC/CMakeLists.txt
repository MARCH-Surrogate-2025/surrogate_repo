set(EXECUTABLE_FILES Test_RC)

add_library(CUSTOM_RC_UTILS SHARED  src/Test_RC.cpp src/EL3068.cpp)
target_link_libraries(CUSTOM_RC_UTILS PUBLIC ${LIBS})

foreach(executable_file ${EXECUTABLE_FILES})

    set(executable_sources src/${executable_file}.cpp)
    set(executable_headers src/${executable_file}.h)

    add_executable(${executable_file} ${executable_sources} ${executable_headers})
    target_link_libraries(${executable_file} PUBLIC ${LIBS} CUSTOM_RC_UTILS)
endforeach ()

