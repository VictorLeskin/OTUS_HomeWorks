///************************* ITELMA SP ****************************************
#ifndef IOC_HPP
#define IOC_HPP

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <stdexcept>

// IoC-container
// with only entry
// T *Resolve(std::string s, Args... args);
// if s is "Register" this means query to register factory or factory method
// Registering a factory is performing by the function 
// iCommand *registerFactory(std::string scope, const cFactory &);
// class cFactory  is a wrapper of std::map<string,void*> by key = object name and 
// void * is a factory method.
// User should run Execute() to perform real registering

class IoC;
class cFactory;

// interface class of command
class iCommand
{
public:
	virtual ~iCommand() = default;

	virtual void Execute() = 0;
	virtual const char* Type() = 0;
};

// base class of exception used in task. Just keep a text of a event.
class cException : public std::exception
{
public:
	cException(const char* sz) : szWhat(sz) {}

	const char* what() const noexcept { return szWhat; }

protected:
	const char* szWhat;
};

struct iRegisterFactory : public iCommand
{
  iRegisterFactory(IoC& ioc, const std::string& scope, const cFactory& f) : ioc(&ioc), scope(scope), f(&f) {}

  void Execute() override;

  const char* Type()  override { return typeid(*this).name(); }

  IoC* ioc;
  const std::string scope;
  const cFactory* f;
};

struct iRegisterFactoryMethod : public iCommand
{
  iRegisterFactoryMethod(IoC& ioc, const std::string& scope, const std::string& objName, const void* f)
    : ioc(&ioc), scope(scope), objName(objName), f(f) {}

  void Execute() override;

  const char* Type()  override { return typeid(*this).name(); }

  IoC* ioc;
  const std::string scope, objName;
  const void* f;
};


// Factory 
// keep factory methond in a map<string,function pointer>
class cFactory
{
  friend struct iRegisterFactoryMethod;

public:
		template< typename T, typename... Args>
		using funcPointer = T * (*)(Args... args);

		// register a factory method
		template< typename T, typename... Args>
		void Register(const std::string& objName, T* (*f)(Args... args))
		{
			doRegister(objName, (const void*)f);
		}

		// get a factory method
		// throw cException if there is not a requested factory method
		template< typename T, typename... Args>
		funcPointer<T,Args...> getFactoryMethod(const std::string& objName) const
		{
			try 
			{
				return funcPointer<T, Args...>(factoryMethods.at(objName));
			}
			catch (const std::out_of_range&) // Ups.... 
			{
				throw cException( "There isn't such factory method.");
			}
		}

    int size() const { return int(factoryMethods.size()); }

	protected:
		// keeps the pointer as old plain pointers.
		void doRegister(const std::string& objName, const void* f)
		{
			factoryMethods[objName] = f;
		}

  protected:
		std::map<std::string, const void*> factoryMethods;
};

// IoC container class for a factory pattern.
// function Resolve with first parameter "Register" registers a factory or factory method of a scope
// and returns pointer to instance of iCommand. The command shold execute to perform real registering like
// IoC t;
// cFactory f;
// extern int *generateInt( double, string );
// f.Register("",.function );
// t.Resolve( "Register", "ScopeName", f )->Execute();
// t.Resolve( "Register", "ScopeName", "get int for me", generateInt )->Execute();

class IoC
{
// class 
protected:

  friend struct iRegisterFactory;
  friend struct iRegisterFactoryMethod;

private:
  IoC(const IoC&) = delete;
  IoC &operator=(const IoC&) = delete;

public:
  IoC() = default;
  ~IoC() = default;

  template< typename T, typename S1, typename S2, typename... Args>
  T* Resolve(S1 s1, S2 s2, Args... args)
  {
    // convert paramters to string ..... 
    return ssResolve<T>(std::string(s1), std::string(s2), std::forward<Args>(args)...);
  }


protected:
  template< typename T, typename... Args>
  /**/iCommand* doRegisterFactoryMethod(const std::string& scope, const std::string& objName, T* (*f)(Args... args))
  {
    return new iRegisterFactoryMethod(*this, scope, objName, (const void *)f);
  }

  template< typename T, typename... Args>
  iCommand* doRegister(const std::string& scope, Args... args)
  {
    return doRegisterFactoryMethod<T,Args...>(scope, std::forward<Args>(args)...);
  }

  // dummy but it works works
  /**/template<> iCommand* doRegister<iCommand, const cFactory&>(const std::string& scope, const cFactory& f)
  {
    return new iRegisterFactory(*this, scope, f);
  }
  /**/template<> iCommand* doRegister<iCommand, const cFactory*>(const std::string& scope, const cFactory* f) { return doRegister<iCommand, const cFactory&>(scope, *f); }
  /**/template<> iCommand* doRegister<iCommand, cFactory>(const std::string& scope, cFactory f) { return doRegister<iCommand, const cFactory&>(scope, f); }
  /**/template<> iCommand* doRegister<iCommand, cFactory&>(const std::string& scope, cFactory& f) { return doRegister<iCommand, const cFactory&>(scope, f); }
  /**/template<> iCommand* doRegister<iCommand, cFactory*>(const std::string& scope, cFactory* f) { return doRegister<iCommand, const cFactory&>(scope, *f); }

  template< typename T, typename... Args>
  void* getMethod(const std::string& s1, const std::string& s2)
  {
    return nullptr;
  }

  template< typename T, typename... Args>
  T* doResolve(const std::string s1, const std::string s2, Args... args)
  {
    auto method = getMethod<T, Args... >(s1, s2);
    using f = T * (*)(Args...);
    return (*f(method))(args...);
  }

  template< typename T, typename... Args>
  T* ssResolve(const std::string s1, const std::string s2, Args... args)
  {
    if (s1 == "Register") 
      return doRegister<iCommand>(s2, std::forward<Args>(args)...);
    else
      return doResolve<T>(s1, s2, std::forward<Args>(args)...);
  }

  

protected:
  std::map<std::string, cFactory> factories;
};


inline void iRegisterFactory::Execute()
{
  ioc->factories[scope] = *f;
}

inline void iRegisterFactoryMethod::Execute()
{
  try
  {
    auto &m = ioc->factories[scope];
    m.doRegister(objName, f);
  }
  catch (const std::out_of_range&) // Ups.... 
  {
    throw cException("There isn't such factory.");
  }
}


#endif //#ifndef IOC_HPP



