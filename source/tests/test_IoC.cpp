///************************* ITELMA SP ****************************************

#include <gtest/gtest.h>

#include "ioc.hpp"

// clang-format off

// gTest grouping class
class test_IoC : public ::testing::Test
{
public:
    // additional class to access to member of tested class
    class Test_Obj
    {
    public:
        Test_Obj(int i, const char* sz) : i(i), sz(sz) {}

        int i;
        const char* sz;
    };


  // additional class to access to member of tested class
  class Test_IoC : public IoC
  {
  public:
    // add here members for free access.
    using IoC::IoC; // delegate constructors
  };

};
 
TEST_F(test_IoC, test_Resolve )
{
  Test_IoC t;
  const char* sz = "Amazing";

  Test_Obj* p = t.Resolve<Test_Obj>(std::string("Test_Obj"), 2, sz );

  EXPECT_EQ(2, p->i);
  EXPECT_EQ(sz, p->sz);
}


int f(double d, int x)
{
  return int( x + d );
}

// helper to convert function pointer as void * to real function 
template<typename T, typename... Args>
T callFunction(void* function, const Args&... args)
{
  return reinterpret_cast<T(*)(Args...)>(function)(args...);
}

TEST_F(test_IoC, test_ConvertFunctionPointer )
{
  void* function = reinterpret_cast<void*>(&f);

  EXPECT_EQ( 4, (callFunction<int, double, int>(function, 1, 3)) );
  EXPECT_EQ( 4, (callFunction<int>(function, 1.0, 3)) );
}


template <int I, class... Ts>
decltype(auto) get_bbbb(Ts&&... ts) {
    return std::get<I>(std::forward_as_tuple(ts...));
}

TEST_F(test_IoC, test_fffff)
{
    auto m = get_bbbb<1>(1.0, 2, "abd");
    EXPECT_EQ(2, m );
}
