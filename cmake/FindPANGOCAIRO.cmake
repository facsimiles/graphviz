include(FindPackageHandleStandardArgs)
find_package(PkgConfig)

pkg_check_modules(PANGOCAIRO pangocairo)

if(MINGW)
  find_package(GLIB)

  find_program(GOBJECT_RUNTIME_LIBRARY NAMES libgobject-2.0-0.dll)
  find_program(HARFBUZZ_RUNTIME_LIBRARY NAMES libharfbuzz-0.dll)
  find_program(PANGO_RUNTIME_LIBRARY NAMES libpango-1.0-0.dll)
  find_program(PANGOCAIRO_RUNTIME_LIBRARY NAMES libpangocairo-1.0-0.dll)
  find_program(PANGOFT_RUNTIME_LIBRARY NAMES libpangoft2-1.0-0.dll)
  find_program(PANGOWIN_RUNTIME_LIBRARY NAMES libpangowin32-1.0-0.dll)

  find_package_handle_standard_args(PANGOCAIRO DEFAULT_MSG
    PANGOCAIRO_INCLUDE_DIRS

    PANGOCAIRO_LIBRARIES

    GLIB_RUNTIME_LIBRARIES
    GOBJECT_RUNTIME_LIBRARY
    HARFBUZZ_RUNTIME_LIBRARY
    PANGO_RUNTIME_LIBRARY
    PANGOCAIRO_RUNTIME_LIBRARY
    PANGOFT_RUNTIME_LIBRARY
    PANGOWIN_RUNTIME_LIBRARY
  )
  set(PANGOCAIRO_RUNTIME_LIBRARIES
    ${GLIB_RUNTIME_LIBRARIES}
    ${GOBJECT_RUNTIME_LIBRARY}
    ${HARFBUZZ_RUNTIME_LIBRARY}
    ${PANGO_RUNTIME_LIBRARY}
    ${PANGOCAIRO_RUNTIME_LIBRARY}
    ${PANGOFT_RUNTIME_LIBRARY}
    ${PANGOWIN_RUNTIME_LIBRARY}
  )
elseif(WIN32)
  find_package(GLIB)

  find_program(FFI_RUNTIME_LIBRARY NAMES ffi-8.dll)
  find_program(FRIBIDI_RUNTIME_LIBRARY NAMES fribidi-0.dll)
  find_program(GIO_RUNTIME_LIBRARY NAMES gio-2.0-0.dll)
  find_program(GMODULE_RUNTIME_LIBRARY NAMES gmodule-2.0-0.dll)
  find_program(GOBJECT_RUNTIME_LIBRARY NAMES gobject-2.0-0.dll)
  find_program(HARFBUZZ_RUNTIME_LIBRARY NAMES harfbuzz.dll)
  find_program(PANGO_RUNTIME_LIBRARY NAMES pango-1.0-0.dll)
  find_program(PANGOCAIRO_RUNTIME_LIBRARY NAMES pangocairo-1.0-0.dll)
  find_program(PANGOFT_RUNTIME_LIBRARY NAMES pangoft2-1.0-0.dll)
  find_program(PANGOWIN_RUNTIME_LIBRARY NAMES pangowin32-1.0-0.dll)

  find_package_handle_standard_args(PANGOCAIRO DEFAULT_MSG
    PANGOCAIRO_INCLUDE_DIRS
    PANGOCAIRO_LIBRARIES
    FFI_RUNTIME_LIBRARY
    FRIBIDI_RUNTIME_LIBRARY
    GIO_RUNTIME_LIBRARY
    GMODULE_RUNTIME_LIBRARY
    GOBJECT_RUNTIME_LIBRARY
    HARFBUZZ_RUNTIME_LIBRARY
    PANGO_RUNTIME_LIBRARY
    PANGOCAIRO_RUNTIME_LIBRARY
    PANGOFT_RUNTIME_LIBRARY
    PANGOWIN_RUNTIME_LIBRARY
  )

  set(PANGOCAIRO_RUNTIME_LIBRARIES
    ${FFI_RUNTIME_LIBRARY}
    ${FRIBIDI_RUNTIME_LIBRARY}
    ${GIO_RUNTIME_LIBRARY}
    ${GLIB_RUNTIME_LIBRARIES}
    ${GMODULE_RUNTIME_LIBRARY}
    ${GOBJECT_RUNTIME_LIBRARY}
    ${HARFBUZZ_RUNTIME_LIBRARY}
    ${PANGO_RUNTIME_LIBRARY}
    ${PANGOCAIRO_RUNTIME_LIBRARY}
    ${PANGOFT_RUNTIME_LIBRARY}
    ${PANGOWIN_RUNTIME_LIBRARY}
  )
else()
  find_package_handle_standard_args(PANGOCAIRO DEFAULT_MSG
    PANGOCAIRO_INCLUDE_DIRS
    PANGOCAIRO_LIBRARIES
    PANGOCAIRO_LINK_LIBRARIES
  )
endif()
