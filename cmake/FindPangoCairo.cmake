include(FindPackageHandleStandardArgs)

if (WIN32)
    find_path(PangoCairo_INCLUDE_DIR pango/pangocairo.h PATH_SUFFIXES pango-1.0)
    find_path(GLIB_INCLUDE_DIR glib.h PATH_SUFFIXES glib-2.0)
    find_path(GLIBCONFIG_INCLUDE_DIR glibconfig.h PATH_SUFFIXES lib/glib-2.0/include)

    find_library(GLIB_LIBRARY NAMES glib-2.0)
    find_library(GOBJECT_LIBRARY NAMES gobject-2.0)
    find_library(PANGO_LIBRARY NAMES pango-1.0)
    find_library(PangoCairo_LIBRARY NAMES pangocairo-1.0)

    find_program(GLIB_RUNTIME_LIBRARY NAMES glib-2.dll)
    find_program(GOBJECT_RUNTIME_LIBRARY NAMES gobject-2.dll)
    find_program(HARFBUZZ_RUNTIME_LIBRARY NAMES libharfbuzz-0.dll)
    find_program(PANGO_RUNTIME_LIBRARY NAMES pango-1.dll)
    find_program(PangoCairo_RUNTIME_LIBRARY NAMES pangocairo-1.dll)
    find_program(PANGOFT_RUNTIME_LIBRARY NAMES pangoft2-1.dll)
    find_program(PANGOWIN_RUNTIME_LIBRARY NAMES pangowin32-1.dll)

    find_package_handle_standard_args(PangoCairo DEFAULT_MSG
        GLIB_INCLUDE_DIR
        GLIBCONFIG_INCLUDE_DIR
        PangoCairo_INCLUDE_DIR

        GLIB_LIBRARY
        GOBJECT_LIBRARY
        PANGO_LIBRARY
        PangoCairo_LIBRARY

        GLIB_RUNTIME_LIBRARY
        GOBJECT_RUNTIME_LIBRARY
        HARFBUZZ_RUNTIME_LIBRARY
        PANGO_RUNTIME_LIBRARY
        PangoCairo_RUNTIME_LIBRARY
        PANGOFT_RUNTIME_LIBRARY
        PANGOWIN_RUNTIME_LIBRARY
    )

    set(PangoCairo_INCLUDE_DIRS
        ${GLIB_INCLUDE_DIR}
        ${GLIBCONFIG_INCLUDE_DIR}
        ${PangoCairo_INCLUDE_DIR}
    )

    set(PangoCairo_LIBRARIES
        glib-2.0
        gobject-2.0
        pango-1.0
        pangocairo-1.0
    )

    set(PangoCairo_LINK_LIBRARIES
        ${GLIB_LIBRARY}
        ${GOBJECT_LIBRARY}
        ${PANGO_LIBRARY}
        ${PangoCairo_LIBRARY}
    )

    set(PangoCairo_RUNTIME_LIBRARIES
        ${GLIB_RUNTIME_LIBRARY}
        ${GOBJECT_RUNTIME_LIBRARY}
        ${HARFBUZZ_RUNTIME_LIBRARY}
        ${PANGO_RUNTIME_LIBRARY}
        ${PangoCairo_RUNTIME_LIBRARY}
        ${PANGOFT_RUNTIME_LIBRARY}
        ${PANGOWIN_RUNTIME_LIBRARY}
    )
else()
    find_package(PkgConfig)
    pkg_check_modules(PangoCairo pangocairo)

    # prior to CMake 3.12, PkgConfig does not set *_LINK_LIBRARIES, so do it
    # manually
    if (${CMAKE_VERSION} VERSION_LESS 3.12)
        find_library(PangoCairo_LINK_LIBRARIES pangocairo-1.0
          PATHS ENV LIBRARY_PATH)
    endif()

    find_package_handle_standard_args(PangoCairo DEFAULT_MSG
        PangoCairo_INCLUDE_DIRS
        PangoCairo_LIBRARIES
        PangoCairo_LINK_LIBRARIES
    )
endif()
