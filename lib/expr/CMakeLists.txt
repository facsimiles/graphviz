BISON_TARGET(
  Exparse
  ${CMAKE_CURRENT_SOURCE_DIR}/exparse.y
  ${CMAKE_CURRENT_BINARY_DIR}/exparse.c
  COMPILE_FLAGS "--yacc -Wno-yacc --defines --debug --verbose"
)

add_library(expr STATIC
  # Header files
  exgram.h
  exlib.h
  exop.h
  expr.h

  # Source files
  excc.c
  excontext.c
  exdata.c
  exerror.c
  exeval.c
  exexpr.c
  exopen.c
  extoken.c
  extype.c
  exzero.c
  exnospace.c

  # Generated files
  ${BISON_Exparse_OUTPUTS}
)

target_include_directories(expr PRIVATE
  ..
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/.. # needed to find generated expr/exparse.h
  ../sfio
  ../cdt
)
