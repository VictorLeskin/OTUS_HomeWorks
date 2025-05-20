///************************* ITELMA SP ****************************************
#ifndef IOC_HPP
#define IOC_HPP

#include <string>
#include <memory>
#include <functional>
#include <map>

// IoC-container
// with only entry
// T *Resolve(std::string s, Args... args);
// if s is "Register" this means query to register factory or factory method
// Registering a factory is performing by the function 
// iCommand *registerFactory(std::string scope, const cFactory &);
// class cFactory  is a wrapper of std::map<string,void*> by key = object name and 
// void * is a factory method.
// User should run Execute() to perform real registering

class iCommand
{
public:
	virtual void Execute() = 0;
};


class cFactory
{
	public:

		template< typename T, typename... Args>
		using funcPointer = T * (*)(Args... args);

		template< typename T, typename... Args>
		void Register(const std::string& objName, T* (*f)(Args... args))
		{
			doRegister(objName, (const void*)f);
		}

		template< typename T, typename... Args>
		void Register(const std::string& objName, std::function<T*(Args... args)> f)
		{
			auto t = f.target<funcPointer<T, Args...> >();
			doRegister(objName, t);
		}


		template< typename T, typename... Args>
		funcPointer<T,Args...> getFactoryMethod(const std::string& objName) const 
		{
			try 
			{
				const void *&value = map.at(objName);
				return funcPointer<T, Args...>(value);
			}
			catch (const std::out_of_range&) 
			{
				throw cException();
			}
		}

	protected:
		void doRegister(const std::string& objName, const void* f)
		{
			factoryMethods[objName] = f;
		}

  protected:

		std::map<std::string, const void*> factoryMethods;
};

class IoC
{
//protected:
//	std::shared_ptr<iCommand> registerFactory(const std::string& scope, const cFactory&);
//	std::shared_ptr<iCommand> registerFactoryMethod(const std::string& scope, const std::string &objectName, void *);
//
//	template< typename T, typename... Args>
//	std::shared_ptr<iCommand> registerFactoryMethod(const std::string& scope, const std::string& objectName, T *(*Create)(Args... ) );
//
//	std::shared_ptr<iCommand> doRegister(const std::string& scope, const std::string& objectName, void*);
//
//	template< typename T, typename... Args>
//	std::shared_ptr<T> doResolve(std::string s, Args... args);
//
//public:
//	template< typename T, typename... Args>
//	std::shared_ptr<T> Resolve(std::string s, Args... args);
//
//	//return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
//
};


//template<typename T, typename ...Args>
//inline std::shared_ptr<T> IoC::Resolve(std::string s, Args ...args)
//{
//	if( s == "Register")
//		return doRegister( std::forward<Args>(args)...) );
//	else
//		doResolve(s, std::forward<Args>(args)...) );
//}

//template<typename T, typename ...Args>
//inline T* IoC::Register(std::string s, Args ...args)
//{
//	if (std::string("Register") == s)
//		return doRegister(args...)
//	else
//		return Resolve(s, args...);
//	return nullptr;
//}

#endif //#ifndef IOC_HPP
