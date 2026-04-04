if(ANN_FOUND)

  add_library(libmingle STATIC
    agglomerative_bundling.cpp
    edge_bundling.cpp
    ink.cpp
    nearest_neighbor_graph.cpp
    nearest_neighbor_graph_ann.cpp
  )

  target_include_directories(libmingle PRIVATE
    ..
    ../cdt
    ../cgraph
    ../common
    ../gvc
    ../pathplan
  )

  target_include_directories(libmingle SYSTEM PRIVATE
    ${ANN_INCLUDE_DIR}
  )

endif()
