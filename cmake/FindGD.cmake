find_path(GD_INCLUDE_DIR gd.h)
find_library(GD_LIBRARY NAMES gd libgd)

include(FindPackageHandleStandardArgs)
if(WIN32)
  find_program(GD_RUNTIME_LIBRARY libgd.dll)
  if(MINGW)
    find_program(JPEG_RUNTIME_LIBRARY libjpeg-8.dll)
    find_program(TIFF_RUNTIME_LIBRARY libtiff-6.dll)
    find_program(WEBP_RUNTIME_LIBRARY libwebp-7.dll)
    find_program(LZMA_RUNTIME_LIBRARY liblzma-5.dll)
    find_program(SHARPYUV_RUNTIME_LIBRARY libsharpyuv-0.dll)
  else()
    find_program(JPEG_RUNTIME_LIBRARY jpeg62.dll)
    find_program(TIFF_RUNTIME_LIBRARY tiff.dll)
    find_program(WEBP_RUNTIME_LIBRARY libwebp.dll)
    find_program(LZMA_RUNTIME_LIBRARY liblzma.dll)
    find_program(SHARPYUV_RUNTIME_LIBRARY libsharpyuv.dll)
  endif()

  find_package_handle_standard_args(GD DEFAULT_MSG
                                    GD_LIBRARY GD_INCLUDE_DIR
                                    GD_RUNTIME_LIBRARY
                                    JPEG_RUNTIME_LIBRARY
                                    TIFF_RUNTIME_LIBRARY
                                    WEBP_RUNTIME_LIBRARY
                                    LZMA_RUNTIME_LIBRARY
                                    SHARPYUV_RUNTIME_LIBRARY)

  set(GD_RUNTIME_LIBRARIES
    ${GD_RUNTIME_LIBRARY}
    ${JPEG_RUNTIME_LIBRARY}
    ${TIFF_RUNTIME_LIBRARY}
    ${WEBP_RUNTIME_LIBRARY}
    ${LZMA_RUNTIME_LIBRARY}
    ${SHARPYUV_RUNTIME_LIBRARY}
  )
else()
  find_package_handle_standard_args(GD DEFAULT_MSG
                                    GD_LIBRARY GD_INCLUDE_DIR)
endif()

mark_as_advanced(GD_INCLUDE_DIR GD_LIBRARY GD_RUNTIME_LIBRARY)

set(GD_INCLUDE_DIRS ${GD_INCLUDE_DIR})
set(GD_LIBRARIES ${GD_LIBRARY})

find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_check_modules(GDLIB gdlib>=2.0.33)
endif()

if(GD_LIBRARY)
  find_program(GDLIB_CONFIG gdlib-config)
  if(GDLIB_CONFIG)
    message(STATUS "Found gdlib-config: ${GDLIB_CONFIG}")
    execute_process(COMMAND ${GDLIB_CONFIG} --features
                    OUTPUT_VARIABLE GD_FEATURES
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    COMMAND_ERROR_IS_FATAL ANY)
    message(STATUS "Detected GD features: ${GD_FEATURES}")
    string(REPLACE " " ";" GD_FEATURES_LIST ${GD_FEATURES})
    if("GD_PNG" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_PNG 1)
    endif()
    if("GD_JPEG" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_JPEG 1)
    endif()
    if("GD_XPM" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_XPM 1)
    endif()
    if("GD_FONTCONFIG" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_FONTCONFIG 1)
    endif()
    if("GD_FREETYPE" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_FREETYPE 1)
    endif()
    if("GD_GIF" IN_LIST GD_FEATURES_LIST)
      set(HAVE_GD_GIF 1)
    endif()
  elseif(APPLE OR GDLIB_FOUND)
    # At time of writing, Macports does not package libgd. So assume the user
    # obtained this through Homebrew and hard code the options the Homebrew
    # package enables.
    set(HAVE_GD_PNG 1)
    set(HAVE_GD_JPEG 1)
    set(HAVE_GD_FONTCONFIG 1)
    set(HAVE_GD_FREETYPE 1)
    set(HAVE_GD_GIF 1)
  else()
    message(
      WARNING
      "gdlib-config/gdlib pkgconfig not found; skipping feature checks")
  endif()
endif()
