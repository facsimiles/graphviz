find_path(GTS_INCLUDE_DIR gts.h)
find_library(GTS_LIBRARY NAMES gts libgts)

include(FindPackageHandleStandardArgs)
if (WIN32)
    find_program(GTS_RUNTIME_LIBRARY libgts-0-7-5.dll)
    find_package_handle_standard_args(GTS DEFAULT_MSG
                                      GTS_LIBRARY GTS_INCLUDE_DIR GTS_RUNTIME_LIBRARY)
else()
    find_program(GTS_RUNTIME_LIBRARY libgts.so)
    find_package_handle_standard_args(GD DEFAULT_MSG
                                      GTS_LIBRARY GTS_INCLUDE_DIR)
endif()

mark_as_advanced(GTS_INCLUDE_DIR GTS_LIBRARY GTS_RUNTIME_LIBRARY)

set(GTS_INCLUDE_DIRS ${GTS_INCLUDE_DIR})
set(GTS_LIBRARIES ${GTS_LIBRARY})
set(GTS_RUNTIME_LIBRARIES ${GTS_RUNTIME_LIBRARY})
