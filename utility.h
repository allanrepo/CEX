#ifndef __UTILITY__
#define __UTILITY__

#include <string>
#include <sstream>
#include <iostream>
#include <cstdio>

long toLong(const std::string& num);
bool isNumber(const std::string& n);
bool isInteger(const std::string& n);

class CLog
{
private:
	std::stringstream m_sStream;
public:
	CLog()
	{ 
		clear(); 
		immediate = false; 
		enable = true;
	}
	void clear(){ m_sStream.str(std::string()); }
	void flush(){ std::cout << m_sStream.str(); clear(); }
	bool immediate;
	bool enable;

	template <class T>
	CLog& operator << (const T& s)
	{
		if (enable)
		{
			if (immediate){ std::cout << s; }
			else{ m_sStream << s; }
		}
		return *this;
	}

	static std::ostream& endl(std::ostream &o)
	{
		o << std::endl;	
		return o;
	}
};




#endif
