CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/satgen)

ADD_EXECUTABLE(genarith-test genarith-test.cc)
TARGET_LINK_LIBRARIES(genarith-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(graph-coloring-test graph-coloring-test.cc)
TARGET_LINK_LIBRARIES(graph-coloring-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(dadda-gen-test dadda-gen-test.cc)
TARGET_LINK_LIBRARIES(dadda-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(dadda-gen-test-2 dadda-gen-test-2.cc)
TARGET_LINK_LIBRARIES(dadda-gen-test-2 ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(dadda-gen-test-2-c dadda-gen-test-2-c.cc)
TARGET_LINK_LIBRARIES(dadda-gen-test-2-c ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(dadda-last-phase-test dadda-last-phase-test.cc)
TARGET_LINK_LIBRARIES(dadda-last-phase-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(set-cover-test set-cover-test.cc)
TARGET_LINK_LIBRARIES(set-cover-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(spear-parse-check spear-parse-check.cc)
TARGET_LINK_LIBRARIES(spear-parse-check ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(multiply-gen-test multiply-gen-test.cc)
TARGET_LINK_LIBRARIES(multiply-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(shift-gen-test shift-gen-test.cc)
TARGET_LINK_LIBRARIES(shift-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(shift-imm-gen-test shift-imm-gen-test.cc)
TARGET_LINK_LIBRARIES(shift-imm-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(divmod-gen-test divmod-gen-test.cc)
TARGET_LINK_LIBRARIES(divmod-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(divmod-imm-gen-test divmod-imm-gen-test.cc)
TARGET_LINK_LIBRARIES(divmod-imm-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(ite-gen-test ite-gen-test.cc)
TARGET_LINK_LIBRARIES(ite-gen-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(divmod-gen-bltmod divmod-gen-bltmod.cc)
TARGET_LINK_LIBRARIES(divmod-gen-bltmod ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_EXECUTABLE(spear-force-test spear-force-test.cc)
TARGET_LINK_LIBRARIES(spear-force-test ${GLIBMM_LIBRARIES} satutils satgenmodules)

ADD_TEST(genarithtest genarith-test)
ADD_TEST(graph-coloringtest graph-coloring-test)
ADD_TEST(set-covertest set-cover-test)
ADD_TEST(spear-parsetest1 spear-parse-check
    ${PROJECT_SOURCE_DIR}/satgen/tests/sample.sf.expect
    ${PROJECT_SOURCE_DIR}/satgen/tests/sample.sf)
