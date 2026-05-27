#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TBB::tbb" for configuration "Debug"
set_property(TARGET TBB::tbb APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(TBB::tbb PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtbb_debug.so.12.2"
  IMPORTED_SONAME_DEBUG "libtbb_debug.so.12"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbb )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbb "${_IMPORT_PREFIX}/lib/libtbb_debug.so.12.2" )

# Import target "TBB::tbbmalloc" for configuration "Debug"
set_property(TARGET TBB::tbbmalloc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(TBB::tbbmalloc PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtbbmalloc_debug.so.2.2"
  IMPORTED_SONAME_DEBUG "libtbbmalloc_debug.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbbmalloc )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbbmalloc "${_IMPORT_PREFIX}/lib/libtbbmalloc_debug.so.2.2" )

# Import target "TBB::tbbmalloc_proxy" for configuration "Debug"
set_property(TARGET TBB::tbbmalloc_proxy APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(TBB::tbbmalloc_proxy PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "TBB::tbbmalloc"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtbbmalloc_proxy_debug.so.2.2"
  IMPORTED_SONAME_DEBUG "libtbbmalloc_proxy_debug.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbbmalloc_proxy )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbbmalloc_proxy "${_IMPORT_PREFIX}/lib/libtbbmalloc_proxy_debug.so.2.2" )

# Import target "TBB::tbbbind_2_5" for configuration "Debug"
set_property(TARGET TBB::tbbbind_2_5 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(TBB::tbbbind_2_5 PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtbbbind_2_5_debug.so.3.2"
  IMPORTED_SONAME_DEBUG "libtbbbind_2_5_debug.so.3"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbbbind_2_5 )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbbbind_2_5 "${_IMPORT_PREFIX}/lib/libtbbbind_2_5_debug.so.3.2" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
