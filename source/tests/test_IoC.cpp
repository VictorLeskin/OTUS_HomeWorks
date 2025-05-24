///************************* ITELMA SP ****************************************

#include <gtest/gtest.h>

#include "ioc.hpp"
#include <optional>

// clang-format off

// gTest grouping class
class test_cFactory : public ::testing::Test
{
public:
  // additional class to access to member of tested class
  class Test_cFactory : public cFactory
  {
  public:
    // add here members for free access.
    using cFactory::cFactory; // delegate constructors
    using cFactory::factoryMethods;
    using cFactory::doRegister;


    static Test_cFactory* Clone(const Test_cFactory&) { return nullptr; }

    static int resGetInt;
    static int resGetInt2;
    static int* GetInt(int, double) { return &resGetInt;  }
    static int* GetInt2(int, double) { return &resGetInt2; }
    static int* GetInt3(std::string s ) 
    { 
        static int num;
        num = std::stoi(s);
        return &num; 
    }
  };
};

int test_cFactory::Test_cFactory::resGetInt = 1;
int test_cFactory::Test_cFactory::resGetInt2 = 2;


// gTest grouping class
class test_IoC : public ::testing::Test
{
public:
  // additional class to access to member of tested class
  class Test_Obj
  {
  public:
    Test_Obj(int i, const char* sz) : i(i), sz(sz) {}
    Test_Obj(double d) : d(d) {}
    Test_Obj(char c) : c(c) {}

    static Test_Obj* CreateA(char c)
    {
      return new Test_Obj(c);
    }

    std::optional<int> i;
    std::optional<const char*> sz;
    std::optional<double> d;
    std::optional<char> c;
  };


  // additional class to access to member of tested class
  class Test_IoC : public IoC
  {
  public:
    // add here members for free access.
    using IoC::IoC; // delegate constructors

    using IoC::factories;
    using IoC::doRegisterFactoryMethod;
    using IoC::getMethod;

    using IoC::doRegister;
    using IoC::doResolve;
  };
};

TEST_F(test_cFactory, test_ctor)
{
  Test_cFactory t;
  EXPECT_EQ(0,t.factoryMethods.size());
}

TEST_F(test_cFactory, test_doRegister)
{
  Test_cFactory t; 
  int i;

  t.doRegister("t", &i);

  EXPECT_EQ(1, t.factoryMethods.size());
  EXPECT_EQ( &i, t.factoryMethods["t"] );
}

TEST_F(test_cFactory, test_Register)
{
  // test "old style" function pointers
  Test_cFactory t;

  t.Register(std::string("int"), Test_cFactory::GetInt );
  t.Register(std::string("Test_cFactory"), Test_cFactory::Clone  );

  // test lambda. Unfotunatelly a lamnda can't be converted to function pointer automatically
  // I will grow up and fix this problem. Use this one 
  struct cTmp
  {
    static Test_cFactory* F(const Test_cFactory&) { return new Test_cFactory; };
  };

  t.Register(std::string("lambda Test_cFactory"), cTmp::F);
  
  EXPECT_EQ(3, t.factoryMethods.size());
  EXPECT_EQ(&Test_cFactory::GetInt, t.factoryMethods["int"]);
  EXPECT_EQ(&Test_cFactory::Clone, t.factoryMethods["Test_cFactory"]);
  EXPECT_EQ(&cTmp::F, t.factoryMethods["lambda Test_cFactory"]);
}

TEST_F(test_cFactory, test_getFactoryMethod)
{
  // test "old style" function pointers
  Test_cFactory t;

  // test lambda. Unfotunatelly a lamnda can't be converted to function pointer automatically
  // I will grow up and fix this problem. Use this one 
  struct cTmp
  {
    static int* F(const std::string& s) { return nullptr; }
  };

  t.Register(std::string("K"), cTmp::F);
  t.Register("int", Test_cFactory::GetInt);
  t.Register(std::string("Test_cFactory"), Test_cFactory::Clone);

  auto res0 = t.getFactoryMethod<int,const std::string &>(std::string("K"));
  EXPECT_EQ(&cTmp::F, res0);

  auto res1 = t.getFactoryMethod<int, int, double>("int");
  EXPECT_EQ(&Test_cFactory::GetInt, res1);

  auto res11 = t.getFactoryMethod<double, int, double>("int");

  // no such factory method
  try
  {
    t.getFactoryMethod<int, int, double>("int1");
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory method.", expected.what());
  }

}


TEST_F(test_IoC, test_doRegister_Factory )
{
  Test_IoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);
  f.Register("Test_cFactory", test_cFactory::Test_cFactory::Clone);

  std::unique_ptr<iCommand> res( t.doRegister<iCommand, const cFactory&>("A", f) );
  EXPECT_EQ(0, t.factories.size());
  res->Execute();
  EXPECT_EQ(1, t.factories.size());
  auto fptr = t.factories["A"].getFactoryMethod<int, int, double>("int");
  EXPECT_EQ(&test_cFactory::Test_cFactory::GetInt, fptr);
}

TEST_F(test_IoC, test_doRegisterFactoryMethod)
{
  Test_IoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);
  t.doRegister<iCommand, const cFactory&>("A", f)->Execute();

  std::unique_ptr<iCommand> res(t.doRegisterFactoryMethod<test_cFactory::Test_cFactory, const test_cFactory::Test_cFactory&>("A", "Test_cFactory", test_cFactory::Test_cFactory::Clone));
  EXPECT_EQ(1, t.factories["A"].size());
  res->Execute();
  EXPECT_EQ(2, t.factories["A"].size());

  auto fptr = t.factories["A"].getFactoryMethod<test_cFactory::Test_cFactory, const test_cFactory::Test_cFactory&>("Test_cFactory");
  EXPECT_EQ(&test_cFactory::Test_cFactory::Clone, fptr);


  // no such factory method
  try
  {
    std::unique_ptr<iCommand> res(t.doRegisterFactoryMethod<test_cFactory::Test_cFactory, const test_cFactory::Test_cFactory&>("B", "Test_cFactory", test_cFactory::Test_cFactory::Clone));
    res->Execute();
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory.", expected.what());
  }
}

TEST_F(test_IoC, test_getMethod)
{
  Test_IoC t;

  test_cFactory::Test_cFactory f1;
  f1.Register("int", test_cFactory::Test_cFactory::GetInt);
  f1.Register("Test_cFactory", test_cFactory::Test_cFactory::Clone);
  t.doRegister<iCommand, const cFactory&>("A", f1)->Execute();

  test_cFactory::Test_cFactory f2;
  f2.Register("int", test_cFactory::Test_cFactory::GetInt2);
  t.doRegister<iCommand, const cFactory&>("B", f2)->Execute();

  auto m1 = t.getMethod<int>("A", "int");
  EXPECT_EQ((const void*)test_cFactory::Test_cFactory::GetInt, m1);

  auto m2 = t.getMethod<int>("B", "int");
  EXPECT_EQ((const void*)test_cFactory::Test_cFactory::GetInt2, m2);
}

TEST_F(test_IoC, test_doResolve)
{
    Test_IoC t;

    test_cFactory::Test_cFactory f1;
    f1.Register("int", test_cFactory::Test_cFactory::GetInt);
    f1.Register("int3", test_cFactory::Test_cFactory::GetInt3);
    t.doRegister<iCommand, const cFactory&>("A", f1)->Execute();

    test_cFactory::Test_cFactory f2;
    f2.Register("int", test_cFactory::Test_cFactory::GetInt2);
    t.doRegister<iCommand, const cFactory&>("B", f2)->Execute();

    int* m1 = t.doResolve<int>("B", "int", 2, 33.0);
    int* m2 = t.doResolve<int>("A", "int3", std::string("256") );

    EXPECT_EQ( &test_cFactory::Test_cFactory::resGetInt2, m1);
    EXPECT_EQ( 2, *m1);
    EXPECT_EQ( 256, *m2);

    // no such factory method
    try
    {
        int* m1 = t.doResolve<int>("C", "int", 2, 33.0);
        FAIL();
    }
    catch (const std::exception& expected)
    {
        ASSERT_STREQ("There isn't such factory.", expected.what());
    }

    // no such factory method
    try
    {
        int* m1 = t.doResolve<int>("B", "int88", 2, 33.0);
        FAIL();
    }
    catch (const std::exception& expected)
    {
        ASSERT_STREQ("There isn't such factory method.", expected.what());
    }
}

TEST_F(test_IoC, test_Resolve)
{
    Test_IoC t;

    test_cFactory::Test_cFactory f1;
    f1.Register("int", test_cFactory::Test_cFactory::GetInt);

    // registering 
    t.Resolve<iCommand>("Register", "A", f1)->Execute();
    t.Resolve<iCommand>("Register", "A", "int3", test_cFactory::Test_cFactory::GetInt3 )->Execute();

    int* m2 = t.Resolve<int, std::string>("A", "int3", std::string("256"));
    EXPECT_EQ(256, *m2);
}
