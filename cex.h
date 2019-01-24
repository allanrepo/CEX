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
	CLog m_Log;
	CLog m_Result;
	
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
	bool m_bCommand;
	bool m_bCmdHelp;


	void printHelp();
	void printCmdHelp();
	void executeCommand();


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

