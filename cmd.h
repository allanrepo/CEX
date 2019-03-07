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

/*

command list status:
get_head									[done]
cex_version									[done]
get_name									[done]
get_username									[done]
start [-ntimes <loop_count> [-wait <seconds>]] [-nowait]			[done]
program_loaded
program_load_done
get_exp <expression> <display mode>
evx_summary 
evx_summary <site> [on||off]
evx_summary <site, partial, final> <full [on||off]> <clear [on||off]>
evx_summary [clearfinal || clearpartial]
evx_summary output [sublot|lot] [partial|final]
load <program> -display								[done]
unload -wait <t> -nowait -dontsave
- 	'-dontsave' not working because both evxa and cex unload 
	command ignores this flag find work around to implement this

*/

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

/* ------------------------------------------------------------------------------------------
-c <get_name> opt class
------------------------------------------------------------------------------------------ */
class CGetName: public CCmdBase
{
public:
	CGetName():CCmdBase("get_name"){}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <get_username> opt class
------------------------------------------------------------------------------------------ */
class CGetUserName: public CCmdBase
{
public:
	CGetUserName():CCmdBase("get_username"){}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <load> opt class
------------------------------------------------------------------------------------------ */
class CLoad: public CCmdBase
{
public:
	CLoad():CCmdBase("load")
	{
		addOpt( new CArg("-display") );
	}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <unload> opt class
------------------------------------------------------------------------------------------ */
class CUnload: public CCmdBase
{
public:
	CUnload():CCmdBase("unload")
	{
		addOpt( new CArg("-wait") );
		addOpt( new CArg("-nowait") );
		addOpt( new CArg("-dontsave") );
	}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <start> opt class
------------------------------------------------------------------------------------------ */
class CStart: public CCmdBase
{
private:
	bool m_bLoop; // no loop
	int m_nLoop; // execute once	
	int m_nWaitAfterExec; // wait time in <sec> after each execution
	bool m_bExitAfterExec; // -nowait flag; 
	bool m_bWaitAfterExec; // -wait flag

public:
	CStart():CCmdBase("start")
	{
		addOpt( new CArg("-wait") );
		addOpt( new CArg("-nowait") );
		addOpt( new CArg("-ntimes") );
	}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

#endif
