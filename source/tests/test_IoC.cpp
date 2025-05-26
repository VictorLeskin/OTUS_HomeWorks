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
    static int* GetInt(int, double) { return &resGetInt; }
    static int* GetInt2(int, double) { return &resGetInt2; }
    static int* GetInt3(std::string s)
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
class test_cIoC : public ::testing::Test
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
  class Test_cIoC : public cIoC
  {
  public:
    // add here members for free access.
    using cIoC::cIoC; // delegate constructors

    using cIoC::factories;
    using cIoC::doRegisterFactoryMethod;
    using cIoC::getMethod;
    using cIoC::doRegisterFactory;
    using cIoC::doResolve;
    using cIoC::ssResolve;
  };
};

TEST_F(test_cFactory, test_ctor)
{
  Test_cFactory t;
  EXPECT_EQ(0, t.factoryMethods.size());
}

TEST_F(test_cFactory, test_doRegister)
{
  Test_cFactory t;
  int i;

  t.doRegister("t", &i);

  EXPECT_EQ(1, t.factoryMethods.size());
  EXPECT_EQ(&i, t.factoryMethods["t"]);
}

TEST_F(test_cFactory, test_Register)
{
  // test "old style" function pointers
  Test_cFactory t;

  t.Register(std::string("int"), Test_cFactory::GetInt);
  t.Register(std::string("Test_cFactory"), Test_cFactory::Clone);

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

  auto res0 = t.getFactoryMethod<int, const std::string&>(std::string("K"));
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


TEST_F(test_cIoC, test_doRegisterFactory)
{
  Test_cIoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);
  f.Register("Test_cFactory", test_cFactory::Test_cFactory::Clone);

  const cFactory& f1 = f;
  std::unique_ptr<iCommand> res(t.doRegisterFactory("A", f1));
  EXPECT_EQ(0, t.factories.size());
  res->Execute();
  EXPECT_EQ(1, t.factories.size());
  auto fptr = t.factories["A"].getFactoryMethod<int, int, double>("int");
  EXPECT_EQ(&test_cFactory::Test_cFactory::GetInt, fptr);

  // wrong type for registrations
  try
  {
    t.doRegisterFactory("A", 22)->Execute();
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("Wrong registration type.", expected.what());
  }

}


TEST_F(test_cIoC, test_doRegisterFactoryMethod)
{
  Test_cIoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);

  const cFactory& f1 = f;
  t.doRegisterFactory("A", f1)->Execute();

  std::unique_ptr<iCommand> res(t.doRegisterFactoryMethod("A", "Test_cFactory", test_cFactory::Test_cFactory::Clone));
  EXPECT_EQ(1, t.factories["A"].size());
  res->Execute();
  EXPECT_EQ(2, t.factories["A"].size());

  auto fptr = t.factories["A"].getFactoryMethod<test_cFactory::Test_cFactory>("Test_cFactory");
  EXPECT_EQ((void*)&test_cFactory::Test_cFactory::Clone, (void*)fptr);

  // no such factory method
  try
  {
    std::unique_ptr<iCommand> res(t.doRegisterFactoryMethod<test_cFactory::Test_cFactory>("B", "Test_cFactory", test_cFactory::Test_cFactory::Clone));
    res->Execute();
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory.", expected.what());
  }
}


TEST_F(test_cIoC, test_getMethod)
{
  Test_cIoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);
  f.Register("Test_cFactory", test_cFactory::Test_cFactory::Clone);

  const cFactory& f_ = f;
  t.doRegisterFactory("A", f_)->Execute();

  test_cFactory::Test_cFactory f2;
  const cFactory& f2_ = f2;
  f2.Register("int", test_cFactory::Test_cFactory::GetInt2);
  t.doRegisterFactory("B", f2_)->Execute();

  auto m1 = t.getMethod<int>("A", "int");
  EXPECT_EQ((const void*)test_cFactory::Test_cFactory::GetInt, m1);

  auto m2 = t.getMethod<int>("B", "int");
  EXPECT_EQ((const void*)test_cFactory::Test_cFactory::GetInt2, m2);

  // no such factory
  try
  {
    auto m3 = t.getMethod<int>("C", "int");
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory.", expected.what());
  }

  // no such factory method
  try
  {
    auto m3 = t.getMethod<int>("B", "double");
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory method.", expected.what());
  }
}


TEST_F(test_cIoC, test_doResolve)
{
  Test_cIoC t;

  test_cFactory::Test_cFactory f1;
  f1.Register("int", test_cFactory::Test_cFactory::GetInt);
  f1.Register("int3", test_cFactory::Test_cFactory::GetInt3);
  const cFactory& f1_ = f1;
  t.doRegisterFactory("A", f1_)->Execute();

  test_cFactory::Test_cFactory f2;
  f2.Register("int", test_cFactory::Test_cFactory::GetInt2);
  const cFactory& f2_ = f2;
  t.doRegisterFactory("B", f2_)->Execute();

  int* m1 = t.doResolve<int>("B", "int", 2, 33.0);
  int* m2 = t.doResolve<int>("A", "int3", std::string("256"));

  EXPECT_EQ(&test_cFactory::Test_cFactory::resGetInt2, m1);
  EXPECT_EQ(2, *m1);
  EXPECT_EQ(256, *m2);

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

TEST_F(test_cIoC, test_ssResolve)
{
  Test_cIoC t;
  test_cFactory::Test_cFactory f;

  f.Register("int", test_cFactory::Test_cFactory::GetInt);
  f.Register("int3", test_cFactory::Test_cFactory::GetInt3);

  const cFactory& f1 = f;
  std::unique_ptr<iCommand> res(t.ssResolve<iCommand>(std::string("Register"), std::string("A"), f1));
  EXPECT_EQ(0, t.factories.size());
  res->Execute();
  EXPECT_EQ(1, t.factories.size());

  std::unique_ptr<iCommand> res1(t.ssResolve<iCommand>(std::string("Register"), std::string("A"), "Test_cFactory", test_cFactory::Test_cFactory::Clone));
  EXPECT_EQ(2, t.factories["A"].size());
  res1->Execute();
  EXPECT_EQ(3, t.factories["A"].size());

  
  // no such factory method
  try
  {
    t.ssResolve<iCommand>(std::string("Register"), std::string("A"), 22);
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("Wrong registration type.", expected.what());
  }

  int* m2 = t.ssResolve<int>("A", "int3", std::string("256"));
  EXPECT_EQ(256, *m2);

  // no such factory method
  try
  {
    int* m2 = t.ssResolve<int>("A", "int99", std::string("256"));
    FAIL();
  }
  catch (const std::exception& expected)
  {
    ASSERT_STREQ("There isn't such factory method.", expected.what());
  }

}


// gTest grouping class
class test_IoC : public ::testing::Test
{
public:

  // additional class to access to member of tested class
  class Test_IoC : public IoC
  {
  public:
    // add here members for free access.
    using IoC::IoC; // delegate constructors
  };
};

TEST_F(test_IoC, test_Resolve)
{
  Test_IoC t;

  test_cFactory::Test_cFactory f1;

  f1.Register("int", test_cFactory::Test_cFactory::GetInt);

  // registering 
  const cFactory& f11 = f1;
  t.Resolve<iCommand>("Register", "A", f11)->Execute();
  t.Resolve<iCommand>("Register", "A", "int3", test_cFactory::Test_cFactory::GetInt3)->Execute();
  //
  int* m2 = t.Resolve<int>("A", "int3", std::string("256"));
  EXPECT_EQ(256, *m2);

  double* m3 = t.Resolve<double>("A", "int3", std::string("256"), 333, &f1);
  double* m4 = t.Resolve<double>("A", "int3", std::string("256"), 333, &f1);
}
