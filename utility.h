#ifndef __UTILITY__
#define __UTILITY__

#include <string>
#include <sstream>
#include <iostream>
#include <cstdio>

long toLong(const std::string& num);
/*
class CLog
{
private:
	bool m_enable;
public:
	CLog(){ m_enable = true; }
	virtual ~CLog(){}

	bool enable(bool s = true){ m_enable = s; return s; }

	void print(const char* p, bool bEOL = true)
	{
		if (!m_enable) return; 
		std::cout << p;
		if (bEOL) std::cout << std::endl;
	}
	void print(const std::string& s, bool bEOL = true)
	{
		print(s.c_str(), bEOL);
	}

	template <class T>
	CLog& operator << (const T& s)
	{
		if (!m_enable) return *this;
		std::stringstream ss; 
		ss << s;
		std::cout << ss.str(); 

		return *this;
	}

	static std::ostream &endl(std::ostream &o)
	{
		std::cout << std::endl;	
		return o;
	}
};
*/

bool isNumber(const std::string& n);
bool isInteger(const std::string& n);


#endif
