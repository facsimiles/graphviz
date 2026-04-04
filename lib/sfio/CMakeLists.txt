add_library(sfio STATIC
  sfcvt.c
  sfextern.c
  sftable.c
  sfprint.c
  sfvscanf.c
  Sfio_f/_sfslen.c
)

target_include_directories(sfio PRIVATE
  ..
)

if(WIN32 AND NOT MINGW)
  target_include_directories(sfio PRIVATE
    ../../windows/include/unistd
  )
endif()
