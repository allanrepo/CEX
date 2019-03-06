/*
*/

#ifndef __CEX__
#define __CEX__

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <pwd.h> 
#include <tester.h>  
#include <arg.h>

class CCex: public CArg
{
private:
	const std::string getUserName() const;
	CTester& m_Tester;
	CUtil::CLog& m_Log;
	CUtil::CLog& m_Debug;

public:
	CCex(); 	
	virtual ~CCex(){} 

	bool scan(int argc, char **argv);

	// virtual methods from CArg to be overriden
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);	
};



#endif

