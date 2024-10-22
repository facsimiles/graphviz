if(ENABLE_SWIG)
  set(UseSWIG_MODULE_VERSION 2) # cmake-lint: disable=C0103
  include(UseSWIG)

  set_property(SOURCE gv.i PROPERTY CPLUSPLUS ON)

  if(ENABLE_SHARP)
    swig_add_library(gv_sharp
      LANGUAGE csharp
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_sharp PRIVATE DEMAND_LOADING=1)
    set_property(TARGET gv_sharp PROPERTY SWIG_COMPILE_OPTIONS -namespace)
    swig_link_libraries(gv_sharp PRIVATE gvc)

    get_property(gv_sharp_support
      TARGET gv_sharp
      PROPERTY SWIG_SUPPORT_FILES_DIRECTORY
    )
    install(
      TARGETS gv_sharp
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/sharp
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
    install(
      DIRECTORY ${gv_sharp_support}/
      DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/sharp
    )
  endif()

  if(ENABLE_D)
    swig_add_library(gv_d
      LANGUAGE d
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_d PRIVATE DEMAND_LOADING=1)
    swig_link_libraries(gv_d PRIVATE gvc)

    install(
      TARGETS gv_d
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/d
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_GO)
    swig_add_library(gv_go
      LANGUAGE go
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_go PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_go PROPERTY SWIG_COMPILE_OPTIONS -intgosize ${INTGOSIZE}
    )
    if(HAVE_CXX_WUNUSED_FUNCTION)
      set_property(
        TARGET gv_go
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-unused-function
      )
    endif()
    swig_link_libraries(gv_go PRIVATE gvc)

    add_custom_target(gv.go
      COMMAND echo "package gv" >gv.go
      BYPRODUCTS gv.go
      COMMENT "generating gv.go"
    )
    add_dependencies(gv_go gv.go)

    add_custom_target(runtime.h
      COMMAND ${SWIG_EXECUTABLE} -c++ -go -intgosize ${INTGOSIZE}
        -external-runtime runtime.h
      BYPRODUCTS runtime.h
      COMMENT "generating runtime.h"
    )
    add_dependencies(gv_go runtime.h)

    install(
      TARGETS gv_go
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/go
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.go
            ${CMAKE_CURRENT_BINARY_DIR}/runtime.h
      DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/go
    )
  endif()

  if(ENABLE_GUILE)
    swig_add_library(gv_guile
      LANGUAGE guile
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_guile PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_guile
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES ${GUILE_INCLUDE_DIRS}
    )
    if(HAVE_CXX_WUNUSED_PARAMETER)
      set_property(
        TARGET gv_guile
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-unused-parameter
      )
    endif()
    swig_link_libraries(gv_guile PRIVATE gvc ${GUILE_LIBRARIES})

    install(
      TARGETS gv_guile
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/guile
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_JAVA)
    swig_add_library(gv_java
      LANGUAGE java
      SOURCES gv.cpp gv.i gv_builtins.c gv_java_init.c
    )
    target_compile_definitions(gv_java PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_java
      PROPERTY SWIG_COMPILE_OPTIONS -package org.graphviz
    )
    set_property(
      TARGET gv_java
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES
        ${JNI_INCLUDE_DIRS}
    )
    swig_link_libraries(gv_java PRIVATE gvc ${JNI_LIBRARIES})

    install(
      TARGETS gv_java
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/java
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_JAVASCRIPT)
    swig_add_library(gv_javascript
      LANGUAGE javascript
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_javascript PRIVATE DEMAND_LOADING=1)
    set_property(TARGET gv_javascript PROPERTY SWIG_COMPILE_OPTIONS -jsc)
    set_property(
      TARGET gv_javascript
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES
        ${JAVASCRIPTCORE_INCLUDE_DIRS}
    )
    if(HAVE_CXX_WSIGN_COMPARE AND HAVE_CXX_WUNUSED_PARAMETER)
      set_property(
        TARGET gv_javascript
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS
          -Wno-sign-compare -Wno-unused-parameter
      )
    elseif(HAVE_CXX_WSIGN_COMPARE)
      set_property(
        TARGET gv_javascript
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-sign-compare
      )
    elseif(HAVE_CXX_WUNUSED_PARAMETER)
      set_property(
        TARGET gv_javascript
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-unused-parameter
      )
    endif()
    swig_link_libraries(gv_javascript PRIVATE gvc ${JAVASCRIPTCORE_LIBRARIES})

    install(
      TARGETS gv_javascript
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/javascript
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_LUA)
    swig_add_library(gv_lua
      LANGUAGE lua
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_lua PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_lua
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR}
    )
    swig_link_libraries(gv_lua PRIVATE gvc ${LUA_LIBRARIES})

    install(
      TARGETS gv_lua
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/lua
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_PERL)
    swig_add_library(gv_perl
      LANGUAGE perl5
      SOURCES gv.cpp gv.i gv_builtins.c gv_dummy_init.c
    )
    target_compile_definitions(gv_perl PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_perl
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES ${PERL_INCLUDE_PATH}
    )
    if(HAVE_CXX_WUNUSED_PARAMETER)
      set_property(
        TARGET gv_perl
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-unused-parameter
      )
    endif()
    swig_link_libraries(gv_perl PRIVATE gvc ${PERL_LIBRARY})

    install(
      TARGETS gv_perl
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/perl
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_PHP)
    swig_add_library(gv_php
      LANGUAGE php7
      SOURCES gv.cpp gv.i gv_builtins.c gv_php_init.c
    )
    target_compile_definitions(gv_php PRIVATE DEMAND_LOADING=1)
    set_property(
      TARGET gv_php
      PROPERTY SWIG_GENERATED_INCLUDE_DIRECTORIES ${PHP_INCLUDE_DIRS}
    )
    set_property(
      TARGET gv_php
      PROPERTY INCLUDE_DIRECTORIES ${PHP_INCLUDE_DIRS}
    )
    set(PHP_FLAGS "")
    if(HAVE_CXX_WUNUSED_FUNCTION)
      list(APPEND PHP_FLAGS -Wno-unused-function)
    endif()
    if(HAVE_CXX_WUNUSED_LABEL)
      list(APPEND PHP_FLAGS -Wno-unused-label)
    endif()
    if(HAVE_CXX_WUNUSED_PARAMETER)
      list(APPEND PHP_FLAGS -Wno-unused-parameter)
    endif()
    if(HAVE_CXX_WUNUSED_VARIABLE)
      list(APPEND PHP_FLAGS -Wno-unused-variable)
    endif()
    set_property(
      TARGET gv_php
      PROPERTY SWIG_GENERATED_COMPILE_OPTIONS "${PHP_FLAGS}"
    )
    swig_link_libraries(gv_php PRIVATE gvc ${PHP_LIBRARIES})

    install(
      TARGETS gv_php
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/php
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(ENABLE_TCL)
    swig_add_library(gv_tcl
      LANGUAGE tcl
      SOURCES gv.cpp gv.i gv_builtins.c gv_tcl_init.c
    )
    target_compile_definitions(gv_tcl PRIVATE DEMAND_LOADING=1)
    set_property(TARGET gv_tcl PROPERTY SWIG_COMPILE_OPTIONS -namespace)
    target_include_directories(gv_tcl PRIVATE SYSTEM ${TCL_INCLUDE_PATH})
    if(HAVE_CXX_WSHADOW)
      set_property(
        TARGET gv_tcl
        PROPERTY SWIG_GENERATED_COMPILE_OPTIONS -Wno-shadow
      )
    endif()
    swig_link_libraries(gv_tcl PRIVATE gvc tclstubs ${TCL_LIBRARY})

    install(
      TARGETS gv_tcl
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )

    add_custom_target(gv_tcl_pkgindex
      COMMAND
        ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../mkpkgindex.py"
        --file libgv_tcl${CMAKE_SHARED_LIBRARY_SUFFIX}
        --name gv
        --version 0
      DEPENDS ../mkpkgindex.py
      BYPRODUCTS pkgIndex.tcl
      COMMENT "create gv_tcl package index"
    )
    add_dependencies(gv_tcl_pkgindex gv_tcl)
  endif()

  add_custom_target(man-bindings ALL
    DEPENDS gv.3go gv.3guile gv.3java gv.3lua gv.3perl gv.3php gv.3python
      gv.3r gv.3ruby gv.3sharp gv.3tcl
    COMMENT "man pages for language bindings"
  )
  add_custom_command(
    OUTPUT gv.3go gv.3guile gv.3java gv.3lua gv.3perl gv.3php gv.3python gv.3r
      gv.3ruby gv.3sharp gv.3tcl
    COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/gv_doc_writer.py"
    MAIN_DEPENDENCY gv_doc_writer.py
    DEPENDS gv.i
    COMMENT "generate man pages for language bindings"
  )

  if(ENABLE_TCL)
    if(GZIP)
      add_custom_target(man-gv_tcl ALL DEPENDS gv.3tcl.gz
                        COMMENT "gv_tcl man page")
      add_custom_command(
        OUTPUT gv.3tcl.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3tcl
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3tcl.gz"
        MAIN_DEPENDENCY gv.3tcl
        COMMENT "compress gv_tcl man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3tcl.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3tcl
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_SHARP)
    if(GZIP)
      add_custom_target(man-gv_sharp ALL DEPENDS gv.3sharp.gz
                        COMMENT "gv_sharp man page")
      add_custom_command(
        OUTPUT gv.3sharp.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3sharp
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3sharp.gz"
        MAIN_DEPENDENCY gv.3sharp
        COMMENT "compress gv_sharp man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3sharp.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3sharp
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_GO)
    if(GZIP)
      add_custom_target(man-gv_go ALL DEPENDS gv.3go.gz
                        COMMENT "gv_go man page")
      add_custom_command(
        OUTPUT gv.3go.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3go
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3go.gz"
        MAIN_DEPENDENCY gv.3go
        COMMENT "compress gv_go man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3go.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3go
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_GUILE)
    if(GZIP)
      add_custom_target(man-gv_guile ALL DEPENDS gv.3guile.gz
                        COMMENT "gv_guile man page")
      add_custom_command(
        OUTPUT gv.3guile.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3guile
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3guile.gz"
        MAIN_DEPENDENCY gv.3guile
        COMMENT "compress gv_guile man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3guile.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3guile
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_JAVA)
    if(GZIP)
      add_custom_target(man-gv_java ALL DEPENDS gv.3java.gz
                        COMMENT "gv_java man page")
      add_custom_command(
        OUTPUT gv.3java.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3java
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3java.gz"
        MAIN_DEPENDENCY gv.3java
        COMMENT "compress gv_java man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3java.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3java
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_PERL)
    if(GZIP)
      add_custom_target(man-gv_perl ALL DEPENDS gv.3perl.gz
                        COMMENT "gv_perl man page")
      add_custom_command(
        OUTPUT gv.3perl.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3perl
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3perl.gz"
        MAIN_DEPENDENCY gv.3perl
        COMMENT "compress gv_perl man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3perl.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3perl
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_PHP)
    if(GZIP)
      add_custom_target(man-gv_php ALL DEPENDS gv.3php.gz
                        COMMENT "gv_php man page")
      add_custom_command(
        OUTPUT gv.3php.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3php
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3php.gz"
        MAIN_DEPENDENCY gv.3php
        COMMENT "compress gv_php man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3php.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3php
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()

  if(ENABLE_LUA)
    if(GZIP)
      add_custom_target(man-gv_lua ALL DEPENDS gv.3lua.gz
                        COMMENT "gv_lua man page")
      add_custom_command(
        OUTPUT gv.3lua.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout gv.3lua
          >"${CMAKE_CURRENT_BINARY_DIR}/gv.3lua.gz"
        MAIN_DEPENDENCY gv.3lua
        COMMENT "compress gv_lua man page")
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3lua.gz
        DESTINATION ${MAN_INSTALL_DIR}/man3)
    else()
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gv.3lua
        DESTINATION ${MAN_INSTALL_DIR}/man3
      )
    endif()
  endif()
endif()
