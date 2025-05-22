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

    static int* GetInt(int, double) { return nullptr;  }
    static Test_cFactory* Clone(const Test_cFactory &) { return nullptr; }

    static int* GetInt2(int, double) { return nullptr; }
  };
};

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

TEST_F(test_IoC, test_doResolve)
{
  Test_IoC t;

  test_cFactory::Test_cFactory f1;
  f1.Register("int", test_cFactory::Test_cFactory::GetInt);
  f1.Register("Test_cFactory", test_cFactory::Test_cFactory::Clone);
  t.doRegister<iCommand, const cFactory&>("A", f1)->Execute();

  test_cFactory::Test_cFactory f2;
  f2.Register("int", test_cFactory::Test_cFactory::GetInt2);
  t.doRegister<iCommand, const cFactory&>("B", f1)->Execute();

  auto m = t.doResolve<int>("A", "int");
  EXPECT_EQ((const void*)test_cFactory::Test_cFactory::GetInt, m);
}




#if 0
TEST_F(test_IoC, test_ProofOfConcept)
{
  {
    Test_IoC t;
    cFactory* a0 = (cFactory*)(112);
    cFactory& a1 = *a0;
    const cFactory* b0 = a0;
    const cFactory& b1 = *b0;

    using fptrc = int* (*)(const char c);
    fptrc c = reinterpret_cast<fptrc>(789);


    t.Resolve<iCommand>("Register", "Scope1", a0)->Execute();
    t.Resolve<iCommand>("Register", "Scope1", a1)->Execute();
    t.Resolve<iCommand>("Register", "Scope1", b0)->Execute();
    t.Resolve<iCommand>("Register", "Scope1", b1)->Execute();
    t.Resolve<iCommand>("Register", "Scope1", "obj", c)->Execute();

    std::string res = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
    EXPECT_EQ(res, t.strm.str());
  }



  {
    Test_IoC t;
    const cFactory* a = (const cFactory*)(112);

    t.Resolve<iCommand>("Register", "Scope1", a)->Execute();

    std::string res = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
    EXPECT_EQ(res, t.strm.str());
  }

  {
    using fptr = int* (*)(const char c);
    TestOfConcept_IoC t;
    fptr a = reinterpret_cast<fptr>(789);

    t.Resolve<iCommand>(std::string("Register"), std::string("Scope1"), std::string("Object Name"), a)->Execute();

    std::string res = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
    EXPECT_EQ(res, t.strm.str());

    //auto sum = [](double a, double b)
    //{
    //        return a + b;
    //};
    //
    //t.Resolve<iCommand>("Register", "Scope1", "Object Name", sum)->Execute();
    //
    //std::string res1 = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
    //EXPECT_EQ(res1, t.strm.str());

  }

  return;


  //{
  //    TestOfConcept_IoC t;
  //    cFactory* a = (cFactory*)(112);
  //
  //    t.Resolve<iCommand>("Register", "Scope1", 88)->Execute();
  //
  //    std::string res = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
  //    EXPECT_EQ(res, t.strm.str());
  //}



  //{
  //    TestOfConcept_IoC t;
  //
  //    std::shared_ptr<Test_Obj> m = t.Resolve<Test_Obj>("Scope1", "Test_ObjName", 2, "ab");
  //    std::string res = "doResolve,Scope1,Test_ObjName\n";
  //    EXPECT_EQ(res, t.strm.str());
  //}

}

//TEST_F(test_IoC, test_Resolve )
//{
//  Test_IoC t;
//  const char* sz = "Amazing";
//
//  auto p = t.Resolve<Test_Obj>(std::string("Test_Obj"), 2, sz );
//
//  EXPECT_EQ(true, p->i.has_value() ); EXPECT_EQ(2, p->i);
//  EXPECT_EQ(true, p->sz.has_value()); EXPECT_EQ(sz, p->sz);
//  EXPECT_EQ(false, p->d.has_value()); 
//
//  auto q = t.Resolve<Test_Obj>(std::string("Test_Obj"), 232.0 );
//  EXPECT_EQ(false, q->i.has_value()); 
//  EXPECT_EQ(false, q->sz.has_value()); 
//  EXPECT_EQ(true, q->d.has_value());EXPECT_EQ(232.0, q->d);
//}
//
//TEST_F(test_IoC, test_Resolve_With_Register)
//{
//    Test_IoC t;
//    const char* sz = "Amazing";
//
//    auto p = t.Resolve<Test_Obj>(std::string("Register"), std::string("Test_Obj"), Test_Obj::CreateA);
//}



int f(double d, int x)
{
  return int(x + d);
}

// helper to convert function pointer as void * to real function 
template<typename T, typename... Args>
T callFunction(void* function, const Args&... args)
{
  return reinterpret_cast<T(*)(Args...)>(function)(args...);
}

TEST_F(test_IoC, test_ConvertFunctionPointer)
{
  void* function = reinterpret_cast<void*>(&f);

  EXPECT_EQ(4, (callFunction<int, double, int>(function, 1, 3)));
  EXPECT_EQ(4, (callFunction<int>(function, 1.0, 3)));
}


template <int I, class... Ts>
decltype(auto) get_bbbb(Ts&&... ts) {
  return std::get<I>(std::forward_as_tuple(ts...));
}

TEST_F(test_IoC, test_fffff)
{
  auto m = get_bbbb<1>(1.0, 2, "abd");
  EXPECT_EQ(2, m);
}


#endif 