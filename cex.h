/*

[done] load <program> -display
- 	'-display' option either shows bintool or not

unload -wait <t> -nowait -dontsave
- 	if '-wait' is set to 0 (default), command waits forever when unloading program
- 	all test cases are done on -wait and they are fine
- 	all test cases are done on -nowait and they are fine
- 	'-dontsave' not working because both evxa and cex unload command ignores this flag find work around to implement this

[done] get_head 

[done] cex_version

[done] get_name

[done] get_username

[done] start [-ntimes <loop_count> [-wait <seconds>]] [-nowait]

[done] program_loaded

[done] program_load_done

[done] get_exp <expression> <display mode>

[done] evx_summary 
[done] evx_summary <site> [on||off]
[done] evx_summary <site, partial, final> <full [on||off]> <clear [on||off]>

set_exp


*/

#ifndef __CEX__
#define __CEX__


#include <evxa/ProgramControl.hxx>
#include <evxa/StateNotification.hxx>
#include <evxa/EvxioStreamsClient.hxx>
#include <cstdio>
#include "CmdLineArgs.h"
#include <iostream>
#include "utility.h"
  


class CStateNotification : public StateNotification
{
private:
	CLog m_Log;
public:
  	CStateNotification(const TestheadConnection &thc):StateNotification(thc) 
	{
		m_Log.immediate = true;
		m_Log.enable = true;
	}
	virtual ~CStateNotification(){}
	virtual void gemStateChange(const bool linkState, const bool controlState, const bool spoolState, const char *text_msg);
	virtual void programChange(const EVX_PROGRAM_STATE state, const char *text_msg);
	virtual void gemTerminalMessage(const char *pcMessage);	 
};


class CEvxioStreamClient : public EvxioStreamsClient
{
public:
  	CEvxioStreamClient(char *strTesterName, int nHead):EvxioStreamsClient(strTesterName, nHead) {}
	virtual ~CEvxioStreamClient(){}
};

class CCex
{
private:
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
private:
	// logger
	CLog m_Log; // immediate logs that is displayed by default in CEX
	CLog m_Result; // result logs such as error or failure that occured
	CLog m_Debug; // debug logs that makes app more verbose
	
	// command line option manager
	CArg m_Arg;

	// tester objects
  	TesterConnection *m_pTester;
  	TestheadConnection *m_pConn;
	ProgramControl *m_pProgCtrl;
  	CStateNotification *m_pState;
  	CEvxioStreamClient *m_pEvxio;
	int m_nHead;

	// argument properties and states
	bool m_bHelp;
	bool m_bConnect;
	bool m_bCmdHelp;

	// primary command executions
	void printHelp();
	void printCmdHelp();
	void executeCommand();

	// specific command executions
	bool cmdGetHead(const CArg* pCmd);
	bool cmdLoad(const CArg* pCmd);
	bool cmdUnload(const CArg* pCmd);
	bool cmdCexVersion(const CArg* pCmd);
	bool cmdGetName(const CArg* pCmd);
	bool cmdGetUserName(const CArg* pCmd);
	bool cmdStart(const CArg* pCmd);   
	bool cmdProgramLoaded(const CArg* pCmd);   
	bool cmdProgramLoadDone(const CArg* pCmd);   
	bool cmdGetExp(const CArg* pCmd);   
	bool cmdSetExp(const CArg* pCmd);   
	bool cmdSummary(const CArg* pCmd);   
	bool cmdGem(const CArg* pCmd);   



public:
	CCex(int argc = 0, char **argv = 0);
	virtual ~CCex();

	// tester object functions 
	bool connect();
	void disconnect();
	void loop();
	bool scan(int argc, char **argv);
	bool scanCommandParam(int start, int argc, char **argv);
};

#endif

