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
evx_summary 									[done]
evx_summary <site> [on||off]							[done]
evx_summary <site, partial, final> <full [on||off]> <clear [on||off]>		[done]
evx_summary [clearfinal || clearpartial]					[done]
evx_summary output [sublot|lot] [partial|final]					[done]
evx_summary type [prod|ilqa]
execute_flow <type> [-nowait|-wait <seconds>]]
load <program> -display								[done]
unload -wait <t> -nowait -dontsave							- '-dontsave' not working because both evxa and cex unload command ignores this flag find work around to implement this
evx_dfilter [-m <method> | -n <dlog_index>] [<filter>]
evx_dlog_after_failcount <n>
evx_dlog_before_failcount <n>
evx_dlog_clear_methods [ <dlog_index> ]
evx_dlog_failcount <n>
evx_dlog_file_destination <dlog_method_name> <file name>	
evx_dlog_file_freq [-m <method> | -n <dlog_index> ] <file freq> 		[done]
evx_dlog_methods [ <dlog_index> ]						[done]
evx_dlog_sample_rate [-m <method> | -n <dlog_index>] <n>			[done]
evx_dlog_testID [-m <method> | -n <dlog_index>] <string>			[done]
evx_dlog_type [-m <method> | -n <dlog_index> ] <type> 				[done]
program_state
process_status
processes_hung
qa_mode off | inline | 100pct
qa_retest_count <value>
reset_program
reset_tester <options>
restart
sampling on | off [-tnum <test number> [-start_after <int> -interval <int>]]
save
save_as <program name>
*/

/* ------------------------------------------------------------------------------------------
base class for all commands
------------------------------------------------------------------------------------------ */
class CCmdBase: public CArg
{
protected:
	CUtil::CLog& m_Log;
	CUtil::CLog& m_Debug;
	std::list< std::string > m_Args;

public:
	CCmdBase(const std::string& n = ""):CArg(n), m_Log(CTester::instance().m_Log), m_Debug(CTester::instance().m_Debug)
	{
		// all commands have -h[elp] option
		addChild( new CArg("-help") );
	}
	virtual bool scan(std::list< std::string >& Args)
	{
		// store args and let <command> parse them during exec()
		m_Args = Args;
		return true;
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
		addChild( &CTester::instance() );
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
};

/* ------------------------------------------------------------------------------------------
-c <cex_version> opt class
------------------------------------------------------------------------------------------ */
class CCexVersion: public CCmdBase
{
public:
	CCexVersion():CCmdBase("cex_version"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <get_name> opt class
------------------------------------------------------------------------------------------ */
class CGetName: public CCmdBase
{
public:
	CGetName():CCmdBase("get_name"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <get_username> opt class
------------------------------------------------------------------------------------------ */
class CGetUserName: public CCmdBase
{
public:
	CGetUserName():CCmdBase("get_username"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <program_loaded> opt class
------------------------------------------------------------------------------------------ */
class CProgramLoaded: public CCmdBase
{
public:
	CProgramLoaded():CCmdBase("program_loaded"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <program_load_done> opt class
------------------------------------------------------------------------------------------ */
class CProgramLoadDone: public CCmdBase
{
public:
	CProgramLoadDone():CCmdBase("program_load_done"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <load> opt class
------------------------------------------------------------------------------------------ */
class CLoad: public CCmdBase
{
public:
	CLoad():CCmdBase("load")
	{
		addChild( new CArg("-display") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <unload> opt class
------------------------------------------------------------------------------------------ */
class CUnload: public CCmdBase
{
public:
	CUnload():CCmdBase("unload")
	{
		addChild( new CArg("-wait") );
		addChild( new CArg("-nowait") );
		addChild( new CArg("-dontsave") );
	}
	virtual bool exec();
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
		addChild( new CArg("-wait") );
		addChild( new CArg("-nowait") );
		addChild( new CArg("-ntimes") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <get_exp> opt class
------------------------------------------------------------------------------------------ */
class CGetExp: public CCmdBase
{
public:
	CGetExp():CCmdBase("get_exp")
	{
		addChild( new CArg("expression") );
		addChild( new CArg("value") );
		addChild( new CArg("multi_value") );
		addChild( new CArg("multi_range") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_summary> opt class
------------------------------------------------------------------------------------------ */
class CEvxSummary: public CCmdBase
{
private:
	std::string m_strInvalidArg;
	//CArg* Recursive( CArg* pCurr, std::list< std::string>& v, std::list< std::string >::iterator& it );
public:
	CEvxSummary():CCmdBase("evx_summary"), m_strInvalidArg()
	{
		// configure arguments for -command <evx_summary> <site>
		CArg* pSite = new CArg("site");
		pSite->addChild( new CArg("on") );
		pSite->addChild( new CArg("off") );
		addChild( pSite );

		// configure arguments for -command <evx_summary> <partial>
		pSite = new CArg("site");
		pSite->addChild( new CArg("on") );
		pSite->addChild( new CArg("off") );
		CArg* pFull = new CArg("full");	
		pFull->addChild( new CArg("on") );
		pFull->addChild( new CArg("off") );
		CArg* pClear = new CArg("clear");	
		pClear->addChild( new CArg("on") );
		pClear->addChild( new CArg("off") );
		CArg* pPartial = new CArg("partial");
		pPartial->addChild( pFull );
		pPartial->addChild( pClear );
		pPartial->addChild( pSite );
		addChild( pPartial );

		// configure arguments for -command <evx_summary> <final>
		pClear = new CArg("clear");	
		pClear->addChild( new CArg("on") );
		pClear->addChild( new CArg("off") );
		pSite = new CArg("site");
		pSite->addChild( new CArg("on") );
		pSite->addChild( new CArg("off") );
		CArg* pFinal = new CArg("final");
		pFinal->addChild( pClear );
		pFinal->addChild( pSite );
		addChild( pFinal );

		// configure arguments for -command <evx_summary> <output>
		CArg* pOutput = new CArg("output");
		pOutput->addChild( new CArg("lot") );
		pOutput->addChild( new CArg("sublot") );
		pOutput->addChild( new CArg("final") );
		pOutput->addChild( new CArg("partial") );
		addChild( pOutput );

		// add the rest of possible arguments for -command <evx_summary>
		addChild( new CArg("clearpartial") );
		addChild( new CArg("clearfinal") );
		addChild( new CArg("details") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_dlog_methods> opt class
evx_dlog_methods [ <dlog_index> ]
------------------------------------------------------------------------------------------ */
class CDlogMethods: public CCmdBase
{
public:
	CDlogMethods():CCmdBase("evx_dlog_methods"){}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_dlog_file_freq> opt class
evx_dlog_file_freq [-m <method> | -n <dlog_index> ] <file freq> 
------------------------------------------------------------------------------------------ */
class CDlogFileFreq: public CCmdBase
{
public:
	CDlogFileFreq():CCmdBase("evx_dlog_file_freq")
	{
		addChild( new CArg("-n") );
		addChild( new CArg("-m") );
		addChild( new CArg("Lot") );
		addChild( new CArg("SubLot") );
		addChild( new CArg("Wafer") );
		addChild( new CArg("Session") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_dlog_type> opt class
evx_dlog_type [-m <method> | -n <dlog_index> ] <type>
------------------------------------------------------------------------------------------ */
class CDlogType: public CCmdBase
{
public:
	CDlogType():CCmdBase("evx_dlog_type")
	{
		addChild( new CArg("-n") );
		addChild( new CArg("-m") );
		addChild( new CArg("Production") );
		addChild( new CArg("ILQA") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_dlog_sample_rate> opt class
evx_dlog_sample_rate [-m <method> | -n <dlog_index>] <n>
------------------------------------------------------------------------------------------ */
class CDlogSampleRate: public CCmdBase
{
public:
	CDlogSampleRate():CCmdBase("evx_dlog_sample_rate")
	{
		addChild( new CArg("-n") );
		addChild( new CArg("-m") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <evx_dlog_testID> opt class
evx_dlog_testID [-m <method> | -n <dlog_index>] <string>
------------------------------------------------------------------------------------------ */
class CDlogTestID: public CCmdBase
{
public:
	CDlogTestID():CCmdBase("evx_dlog_testID")
	{
		addChild( new CArg("-n") );
		addChild( new CArg("-m") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------
-c <execute_flow> opt class
execute_flow <type> [-nowait|-wait <seconds>]]
------------------------------------------------------------------------------------------ */
class CExecFlow: public CCmdBase
{
public:
	CExecFlow():CCmdBase("evx_dlog_testID")
	{
		addChild( new CArg("-nowait") );
		addChild( new CArg("-wait") );
	}
	virtual bool exec();
};

/* ------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------ */
class CDebug: public CCmdBase
{
public:
	CDebug():CCmdBase("debug"){}
	virtual bool exec();
};


#endif
