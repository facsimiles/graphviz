include(FindPackageHandleStandardArgs)

find_program(PHP_CONFIG php-config)
if(PHP_CONFIG)
  execute_process(
    COMMAND ${PHP_CONFIG} --includes
    OUTPUT_VARIABLE PHP_INCLUDES
    RESULT_VARIABLE PHP_INCLUDES_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(PHP_INCLUDES_ERROR EQUAL 0)
    # the PHP includes are output as “-I/foo -I/bar …” whereas we want
    # them as “/foo;/bar;…”
    string(REPLACE " " ";" PHP_INCLUDES_LIST ${PHP_INCLUDES})
    foreach(inc IN LISTS PHP_INCLUDES_LIST)
      string(REGEX REPLACE "^-I" "" inc ${inc})
      list(APPEND PHP_INCLUDE_DIRS ${inc})
    endforeach()
  endif()
  execute_process(
    COMMAND ${PHP_CONFIG} --ldflags && ${PHP_CONFIG} --libs
    OUTPUT_VARIABLE PHP_LIBRARIES
    RESULT_VARIABLE PHP_LIBRARIES_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT PHP_LIBRARIES_ERROR EQUAL 0)
    unset(PHP_LIBRARIES)
  endif()
endif()

find_package_handle_standard_args(PHP DEFAULT_MSG
  PHP_INCLUDE_DIRS
  PHP_LIBRARIES
)
