add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(neatogen STATIC
  # Header files
  adjust.h
  bfs.h
  call_tri.h
  closest.h
  conjgrad.h
  defs.h
  delaunay.h
  digcola.h
  dijkstra.h
  edges.h
  embed_graph.h
  fPQ.h
  geometry.h
  heap.h
  hedges.h
  info.h
  kkutils.h
  matrix_ops.h
  multispline.h
  neato.h
  neatoprocs.h
  overlap.h
  pca.h
  poly.h
  quad_prog_solver.h
  quad_prog_vpsc.h
  site.h
  sparsegraph.h
  stress.h
  voronoi.h
  sgd.h
  randomkit.h

  # Source files
  adjust.c
  bfs.c
  call_tri.c
  circuit.c
  closest.c
  compute_hierarchy.c
  conjgrad.c
  constrained_majorization.c
  constraint.c
  delaunay.c
  dijkstra.c
  edges.c
  embed_graph.c
  geometry.c
  heap.c
  hedges.c
  info.c
  kkutils.c
  legal.c
  lu.c
  matinv.c
  matrix_ops.c
  multispline.c
  neatoinit.c
  neatosplines.c
  opt_arrangement.c
  overlap.c
  pca.c
  poly.c
  quad_prog_solve.c
  site.c
  smart_ini_x.c
  solve.c
  stuff.c
  stress.c
  voronoi.c
  sgd.c
  randomkit.c
)

if(with_ipsepcola)
  target_sources(neatogen PRIVATE
    constrained_majorization_ipsep.c
    quad_prog_vpsc.c
  )
endif()

target_include_directories(neatogen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(neatogen PRIVATE
  cgraph
  gvc
  pathplan
  sparse
  rbtree
  util
)

if(with_ipsepcola)
  target_link_libraries(neatogen PRIVATE vpsc)
endif()

if(GTS_FOUND)
  target_include_directories(neatogen SYSTEM PRIVATE
    ${GTS_INCLUDE_DIRS}
  )

  target_link_libraries(neatogen PRIVATE
    ${GTS_LINK_LIBRARIES}
  )

  if(WIN32 AND NOT MINGW AND install_win_dependency_dlls)
    install(
      FILES ${GTS_RUNTIME_LIBRARIES}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  endif()
endif()
