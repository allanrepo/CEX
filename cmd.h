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
general help
------------------------------------------------------------------------------------------ */
class CHelp: public CCmdBase
{
public:
	CHelp(): CCmdBase("-help"){};
	virtual bool exec();
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

/* ------------------------------------------------------------------------------------------
-c <cex_version> opt class
------------------------------------------------------------------------------------------ */
class CCexVersion: public CCmdBase
{
public:
	CCexVersion():CCmdBase("cex_version"){}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};


#endif
