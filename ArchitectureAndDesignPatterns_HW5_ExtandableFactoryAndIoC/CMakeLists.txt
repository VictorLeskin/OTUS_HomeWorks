cmake_minimum_required(VERSION 3.14)
project(ArchitectureAndDesignPatterns_HW5_ExtandableFactoryAndIoC)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

add_executable(
  ExtandableFactoryAndIoC

  extandablefactoryandioc.hpp
  extandablefactoryandioc.cpp

  tests/test_ExtandableFactoryAndIoC.cpp

  ../source/ioc.hpp
  ../source/ioc.cpp

  ../source/tests/test_IoC.cpp
)

target_include_directories(
   ExtandableFactoryAndIoC PRIVATE 
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../source
)

target_link_libraries(
  ExtandableFactoryAndIoC
  GTest::gtest_main
  GTest::gmock_main
)

include(GoogleTest GoogleMock)
gtest_discover_tests(ExtandableFactoryAndIoC)