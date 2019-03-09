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
program_loaded									[done]
program_load_done								[done]
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
-c <program_loaded> opt class
------------------------------------------------------------------------------------------ */
class CProgramLoaded: public CCmdBase
{
public:
	CProgramLoaded():CCmdBase("program_loaded"){}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <program_load_done> opt class
------------------------------------------------------------------------------------------ */
class CProgramLoadDone: public CCmdBase
{
public:
	CProgramLoadDone():CCmdBase("program_load_done"){}
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

/* ------------------------------------------------------------------------------------------
-c <get_exp> opt class
------------------------------------------------------------------------------------------ */
class CGetExp: public CCmdBase
{
public:
	CGetExp():CCmdBase("get_exp")
	{
		addOpt( new CArg("expression") );
		addOpt( new CArg("value") );
		addOpt( new CArg("multi_value") );
		addOpt( new CArg("multi_range") );
	}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

/* ------------------------------------------------------------------------------------------
-c <evx_summary> opt class
------------------------------------------------------------------------------------------ */
class CEvxSummary: public CCmdBase
{
private:
	//CArg* Recursive( CArg* pCurr, std::list< std::string>& v, std::list< std::string >::iterator& it );
public:
	CEvxSummary():CCmdBase("evx_summary")
	{
		// cofigure arguments for -command <evx_summary> <site>
		CArg* pSite = new CArg("site");
		pSite->addOpt( new CArg("on") );
		pSite->addOpt( new CArg("off") );

		// cofigure arguments for -command <evx_summary> <partial>
		CArg* pFull = new CArg("full");	
		pFull->addOpt( new CArg("on") );
		pFull->addOpt( new CArg("off") );
		CArg* pClear = new CArg("clear");	
		pClear->addOpt( new CArg("on") );
		pClear->addOpt( new CArg("off") );
		CArg* pPartial = new CArg("partial");
		pPartial->addOpt( pFull );
		pPartial->addOpt( pClear );
		pPartial->addOpt( pSite );

		// configure arguments for -command <evx_summary> <final>
		CArg* pFinal = new CArg("final");
		pClear = new CArg("clear");	
		pClear->addOpt( new CArg("on") );
		pClear->addOpt( new CArg("off") );
		pFinal->addOpt( pClear );
		pFinal->addOpt( pSite );

		// configure arguments for -command <evx_summary> <output>
		CArg* pOutput = new CArg("output");
		pOutput->addOpt( new CArg("lot") );
		pOutput->addOpt( new CArg("sublot") );
		pOutput->addOpt( new CArg("final") );
		pOutput->addOpt( new CArg("partial") );

		// add arguments for -command <evx_summary>
		addOpt( pSite );
		addOpt( pPartial );
		addOpt( pFinal );
		addOpt( pOutput );
		addOpt( new CArg("clearpartial") );
		addOpt( new CArg("clearfinal") );
		addOpt( new CArg("details") );

	}
	virtual bool exec();
	virtual bool scan(std::list< std::string >& Args);
};

#endif
