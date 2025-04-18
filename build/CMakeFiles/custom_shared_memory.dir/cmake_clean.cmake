file(REMOVE_RECURSE
  "libcustom_shared_memory.a"
  "libcustom_shared_memory.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/custom_shared_memory.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
