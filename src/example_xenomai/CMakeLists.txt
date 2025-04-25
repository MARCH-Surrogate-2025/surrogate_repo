set(EXECUTABLE_FILES
    hello_xenomai
    hello_rt_task
    hello_grpc
)

foreach(executable_file ${EXECUTABLE_FILES})
    add_executable(${executable_file} src/${executable_file}.cpp src/${executable_file}.h)
    target_link_libraries(${executable_file} ${LIBS})
endforeach ()

