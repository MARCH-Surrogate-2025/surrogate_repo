set(EXECUTABLE_FILES Test_Dynamixel)

add_library(CUSTOM_DMX_UTILS SHARED src/CustomDynamixel.cpp src/EL6021.cpp)
target_link_libraries(CUSTOM_DMX_UTILS PUBLIC ${LIBS})

foreach(executable_file ${EXECUTABLE_FILES})

    set(executable_sources src/${executable_file}.cpp)
    set(executable_headers src/${executable_file}.h)

    add_executable(${executable_file} ${executable_sources} ${executable_headers})
    target_link_libraries(${executable_file} PUBLIC ${LIBS} CUSTOM_DMX_UTILS)
endforeach ()

