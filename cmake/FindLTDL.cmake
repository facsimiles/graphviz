if(NOT WIN32 OR MINGW)
  find_path(LTDL_INCLUDE_DIR ltdl.h)
  find_library(LTDL_LIBRARY NAMES ltdl)
else()
  find_path(LTDL_INCLUDE_DIR ltdl.h
            PATHS ${CMAKE_CURRENT_SOURCE_DIR}/windows/include/ltdl)
endif()

include(FindPackageHandleStandardArgs)
if(NOT WIN32 OR MINGW)
  find_package_handle_standard_args(LTDL DEFAULT_MSG
                                    LTDL_LIBRARY LTDL_INCLUDE_DIR)
else()
  find_package_handle_standard_args(LTDL DEFAULT_MSG
                                    LTDL_INCLUDE_DIR)
endif()

mark_as_advanced(LTDL_INCLUDE_DIR LTDL_LIBRARY)

set(LTDL_INCLUDE_DIRS ${LTDL_INCLUDE_DIR})
if(NOT WIN32 OR MINGW)
  set(LTDL_LIBRARIES ${LTDL_LIBRARY})
endif()
