///************************* ITELMA SP ****************************************
#ifndef IOC_HPP
#define IOC_HPP

#include <string>

class IoC
{
public:
	template< typename T, typename... Args>
	T* Resolve(std::string& s, Args... args);
};


template<typename T, typename ...Args>
inline T* IoC::Resolve(std::string& s, Args ...args)
{
	return new T( args... );
}

#endif //#ifndef IOC_HPP
