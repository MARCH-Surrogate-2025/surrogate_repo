set(EXECUTABLE_FILES
    test_integration
)


foreach(executable_file ${EXECUTABLE_FILES})
    add_executable(${executable_file} src/${executable_file}.cpp src/${executable_file}.h src/MarchSurrogate.cpp src/MarchSurrogate.h)
    target_link_libraries(${executable_file} ${LIBS} CUSTOM_DMX_UTILS)
endforeach ()

