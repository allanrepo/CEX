#ifndef __CMD__
#define __CMD__

#include <evxa/ProgramControl.hxx>
#include <evxa/StateNotification.hxx>
#include <evxa/EvxioStreamsClient.hxx>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <arg.h>
#include <cex.h>


/* ------------------------------------------------------------------------------------------
general help
------------------------------------------------------------------------------------------ */
class CHelp: public CArg
{
public:
	CHelp(): CArg("-help"){};
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
base class for all commands
------------------------------------------------------------------------------------------ */
class CCmdBase: public CArg
{
protected:
	CUtil::CLog& m_Log;
	CUtil::CLog& m_Debug;

public:
	CCmdBase(const std::string& n = ""):CArg(n), m_Log(CTester::instance().m_Log), m_Debug(CTester::instance().m_Debug)
	{
		// all commands have -h[elp] option
		addOpt( new CArg("-help") );
	}
};

/* ------------------------------------------------------------------------------------------
-c[ommand] opt class
------------------------------------------------------------------------------------------ */
class CCmd: public CCmdBase
{
public:
	CCmd():CCmdBase("-command")
	{
		addOpt( &CTester::instance() );
	} 
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <get_head> opt class
------------------------------------------------------------------------------------------ */
class CGetHead: public CCmdBase
{
public:
	CGetHead():CCmdBase("get_head"){}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};



#endif
