add_library(ast STATIC
  chresc.c
  chrtoi.c
  error.c
  fmtesc.c
  stresc.c
  strmatch.c
)

target_include_directories(ast PRIVATE
  ..
)
