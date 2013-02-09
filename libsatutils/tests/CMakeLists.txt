CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_DEFINITIONS(-DTEST_DIR=\"${PROJECT_SOURCE_DIR}/libsatutils/tests\")

ADD_EXECUTABLE(satio-test satio-test.cc)
TARGET_LINK_LIBRARIES(satio-test ${GLIBMM_LIBRARIES} satutils)

ADD_TEST(satiotest satio-test)
