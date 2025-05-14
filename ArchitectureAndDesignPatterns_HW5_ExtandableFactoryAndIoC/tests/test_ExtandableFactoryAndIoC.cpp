///************************* ITELMA SP ****************************************

#include <gtest/gtest.h>

#include "extandablefactoryandioc.hpp"

// clang-format off

// gTest grouping class
class test_ExtandableFactoryAndIoC : public ::testing::Test
{
public:
  // additional class to access to member of tested class
  class Test_ExtandableFactoryAndIoC : public ExtandableFactoryAndIoC
  {
  public:
    // add here members for free access.
    using ExtandableFactoryAndIoC::ExtandableFactoryAndIoC; // delegate constructors
  };

};
 
TEST_F(test_ExtandableFactoryAndIoC, test_ctor )
{
  Test_ExtandableFactoryAndIoC t;
}

