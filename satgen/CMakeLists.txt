CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_DEFINITIONS(${GLIBMM_CFLAGS})
LINK_DIRECTORIES(${GLIBMM_LIBRARY_DIRS})

ADD_LIBRARY(satgenmodules STATIC
    module.cc
    gen-arith.cc
    parse-graph.cc
    eternity.cc
    libsokobie.cc
    graph-coloring.cc
    subset-sum.cc
    npuzzle.cc
    vertex-cover.cc
    set-cover.cc
    sokoban.cc
    spear-format.cc)

ADD_EXECUTABLE(satgen
    satgen.cc
    modules-table.cc)

TARGET_LINK_LIBRARIES(satgen ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_SUBDIRECTORY(tests)

INSTALL(TARGETS satgen
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
