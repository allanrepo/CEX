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
public:
  	CStateNotification(const TestheadConnection &thc):StateNotification(thc) {}
	virtual ~CStateNotification(){}
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
	class CError
	{
	public:
		std::stringstream m_sError;
		CError(){ clear(); }
		void clear(){ m_sError.str(std::string()); }
		bool state(){ return m_sError.str().length(); }
		void flush(){ std::cout << m_sError.str(); clear(); }

		template <class T>
		CError& operator << (const T& s)
		{
			m_sError << s;
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
	CLog m_Log;
	CLog m_Error;
	CError m_Err;
	
	// command line option manager
	CCmdLineArgs m_Args;

	// tester objects
  	TesterConnection *m_pTester;
  	TestheadConnection *m_pConn;
	ProgramControl *m_pProgCtrl;
  	CStateNotification *m_pState;
  	CEvxioStreamClient *m_pEvxio;
	int m_nHead;

	// argument properties and states
	bool bHelp;


public:
	CCex(int argc = 0, char **argv = 0);
	virtual ~CCex();

	// tester object functions 
	bool connect();
	void disconnect();
	void loop();
	void handleTesterInput(const std::string& strInput);
	bool scan(int argc, char **argv);
};

/*
Design Philosophy
- properties of a command
1. command name 
	- argument option name
2. command arguments 
	- parameters passed a long 'command' option
	- can have 1 or more 
	
*/

#endif

