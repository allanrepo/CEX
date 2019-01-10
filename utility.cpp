#include "utility.h"


long toLong(const std::string& n)
{
	std::stringstream a(n);
	long val;
	a >> val;    
	return val;
} 
