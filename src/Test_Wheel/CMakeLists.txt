set(EXECUTABLE_FILES
        Test_Wheel
)

add_library(CUSTOM_PDO SHARED utils/CustomECAT/PDO.h)
set_target_properties(CUSTOM_PDO PROPERTIES LINKER_LANGUAGE CXX)

add_library(CUSTOM_ECAT_UTILS SHARED utils/CustomECAT/CustomEtherCAT.cpp)
target_link_libraries(CUSTOM_ECAT_UTILS PUBLIC ${LIBS})

foreach(executable_file ${EXECUTABLE_FILES})

    set(executable_sources src/${executable_file}.cpp)
    set(executable_headers src/${executable_file}.h)

    add_executable(${executable_file} ${executable_sources} ${executable_headers})
    target_link_libraries(${executable_file} PUBLIC ${LIBS} CUSTOM_ECAT_UTILS)
endforeach ()

