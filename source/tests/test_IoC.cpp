///************************* ITELMA SP ****************************************

#include <gtest/gtest.h>

#include "ioc.hpp"
#include <optional>

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
        Test_Obj(double d) : d(d) {}
        Test_Obj(char c) : c(c) {}

        static Test_Obj *CreateA(char c)
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
  };

  class TestOfConcept_Command : public iCommand
  {
  public:
      TestOfConcept_Command(std::ostringstream& strm, const char* what) : strm( &strm), what(what)
      {
      }

      void Execute() override
      {
          *strm << "Executing:" << what << std::endl;
      }

      std::ostringstream *strm;
      const char* what;
  };

  class TestOfConcept_IoC
  {
#if 0
  protected:
       // just register a whole factory for the scope
      //iCommand *doRegister(const std::string& scope, const cFactory&f)
      //{
      //    strm << "doRegister," << scope << "," << reinterpret_cast<int>(&f) << std::endl;
      //    return nullptr;
      //}
      //
      //iCommand* doRegister(const std::string& scope, const cFactory* f)
      //{
      //    strm << "doRegister," << scope << "," << reinterpret_cast<int>(&f) << std::endl;
      //    return nullptr;
      //}
      //
      //iCommand* doRegister(const std::string& scope, cFactory* f)
      //{
      //    strm << "doRegister," << scope << "," << reinterpret_cast<int>(&f) << std::endl;
      //    return nullptr;
      //}

      iCommand* doRegister(const std::string& scope, const cFactory& f);
      iCommand* doRegister(const std::string& scope, const cFactory* f);
      iCommand* doRegister(const std::string& scope, cFactory& f);
      iCommand* doRegister(const std::string& scope, cFactory* f);

      // just register a factory method for the scope
      template< typename T, typename ObjType, typename... Args>
      iCommand* doRegister(const std::string& scope, const std::string& nameFactoryMethod, ObjType*(*f)(Args... args) )
      {
          strm << "doRegister," << scope << "," << nameFactoryMethod << "," << reinterpret_cast<int>(f) << std::endl;
          assert(false);
          return nullptr;
      }




      //// just register a factory method for the scope
      //template< typename T, typename... Args>
      //iCommand* doRegister(const std::string& scope, const char* nameFactoryMethod, T* (*f)(Args... args))
      //{
      //    strm << "doRegister," << scope << "," << nameFactoryMethod << "," << reinterpret_cast<int>(f) << std::endl;
      //    assert(false);
      //    return nullptr;
      //}
      //
      //
      //// alloed only two above methond to register generators 
      //template< typename T, typename... Args>
      //iCommand* doRegister(const std::string& s, Args... args)
      //{
      //    throw std::exception("Bad generator.");
      //    return nullptr;
      //}

      template< typename T, typename... Args>
      T *doResolve(const std::string &scope, const std::string &nameFactoryMethod, Args... args)
      {
          // looking for factory
          // looking for factory method
          assert(false);
          return nullptr;
      }

      template< typename T, typename... Args>
      T* ssResolve(const std::string s1, const std::string s2, Args... args)
      {
          if (s1 == "Register")
              return doRegister<iCommand>(s2, std::forward<Args>(args)...);
          else
              return doResolve<T>(s1, s2, std::forward<Args>(args)...);

          return nullptr;
      }

      template< typename T>
      iCommand* ttdoRegister(const std::string& scope, T& t)
      {
          return nullptr;
      }

      template<> iCommand* ttdoRegister(const std::string& scope, std::tuple<const cFactory*>& t);


      template< typename R, typename T>
      R* ttResolve(const std::string s1, const std::string s2, T&& t)
      {
          if (s1 == "Register")
              return ttdoRegister<iCommand>(s2, t);
          //else
          //    return doResolve<T>(s1, s2, std::forward<Args>(args)...);

          return nullptr;
      }



      //template< typename T, typename S1, typename S2, typename... Args>
      //T* Resolve(S1 s1, S2 s2, Args... args)
      //{
      //    //return ssResolve<T>(std::string(s1), std::string(s2), std::forward<Args>(args)...);
      //    return ttResolve<T>(std::string(s1), std::string(s2), std::make_tuple(std::forward<Args>(args)...) );
      //}


#endif

      template< typename T, typename... Args>
      iCommand* doRegisterFactoryMethod(const std::string& scope, const std::string& objName, T* (*)(Args... args))
      {
          return nullptr;
      }
      
      template< typename T, typename... Args>
      iCommand* doRegister(const std::string& scope, Args... args) 
      { 
          return doRegisterFactoryMethod( scope, std::forward<Args>(args)...);
      }
      
      template<> iCommand* doRegister<iCommand,const cFactory&>(const std::string& scope, const cFactory& f) { return nullptr; }
      template<> iCommand* doRegister<iCommand,const cFactory*>(const std::string& scope, const cFactory* f) { return nullptr; }
      template<> iCommand* doRegister<iCommand, cFactory>(const std::string& scope, cFactory f) { return nullptr; }
      template<> iCommand* doRegister<iCommand,cFactory&>(const std::string& scope, cFactory& f) { return nullptr; }
      template<> iCommand* doRegister<iCommand,cFactory*>(const std::string& scope, cFactory* f) { return nullptr; }

      template< typename T, typename... Args>
      void *getMethod(const std::string &s1, const std::string &s2)
      {
          using f = T * (*)(Args...);
          return (f)nullptr;
      }
        

      template< typename T, typename... Args>
      T* doResolve(const std::string s1, const std::string s2, Args... args)
      {
          auto method = getMethod<T, Args... >( s1, s2 );
          using f = T * (*)(Args...);
          return (*f(method))(args...);
      }


      template< typename T, typename... Args>
      T* ssResolve( const std::string s1, const std::string s2, Args... args)
      {
          if (s1 == "Register")
              return doRegister<iCommand>(s2, std::forward<Args>(args)...);
          else
              return doResolve<T>(s1, s2, std::forward<Args>(args)...);

          return nullptr;
      }


  public:

      template< typename T, typename S1, typename S2, typename... Args>
      T* Resolve( S1 s1, S2 s2, Args... args)
      {
          return ssResolve<T>(std::string(s1), std::string(s2), std::forward<Args>(args)...);
          //return ssResolve<T>(std::string(s1), std::string(s2), std::forward<Args>(args)...);
      }



  public:
      std::ostringstream strm;
  };
};

TEST_F(test_IoC, test_ProofOfConcept)
{
    {
        TestOfConcept_IoC t;
        cFactory* a0 = (cFactory*)(112);
        cFactory& a1 = *a0;
        const cFactory* b0 = a0;
        const cFactory &b1 = *b0;

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
        TestOfConcept_IoC t;
        const cFactory* a = (const cFactory*)(112);
    
        t.Resolve<iCommand>("Register", "Scope1", a)->Execute();
    
        std::string res = "doRegister,Scope1,Object Name,789\nExecuting:doRegister\n";
        EXPECT_EQ(res, t.strm.str());
    }
    
    {
        using fptr = int*(*)(const char c);
        TestOfConcept_IoC t;
        fptr a = reinterpret_cast<fptr>(789);

        t.Resolve<iCommand>( std::string( "Register" ), std::string("Scope1"), std::string("Object Name"), a)->Execute();

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
