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

class CTester
{
private:
	// tester objects
  	TesterConnection *m_pTester;
  	TestheadConnection *m_pConn;
	ProgramControl *m_pProgCtrl;
  	CStateNotification *m_pState;
  	CEvxioStreamClient *m_pEvxio;
	int m_nHead;
	
	CLog m_Log;
	void (* m_pTesterInputListener) (const std::string& strInput);

public:
	CTester();
	virtual ~CTester();

	bool connect(const std::string& strTesterName);
	void disconnect();
	void loop();

	void setTesterInputListener( void (* p)(const std::string& strInput) ){ m_pTesterInputListener = p; }	

	// set/get head
	const int head(){ return m_nHead; }
	void head(int h){ m_nHead = h; }
};

class CCex
{
private:
	// logger
	CLog m_Log;
	CLog m_Error;
	
	// command line option manager
	CCmdLineArgs m_Args;

	// tester objects
  	TesterConnection *m_pTester;
  	TestheadConnection *m_pConn;
	ProgramControl *m_pProgCtrl;
  	CStateNotification *m_pState;
  	CEvxioStreamClient *m_pEvxio;
	int m_nHead;


public:
	CCex(int argc = 0, char **argv = 0);
	virtual ~CCex();

	// tester object functions 
	bool connect();
	void disconnect();
	void loop();
	void handleTesterInput(const std::string& strInput);
};

#endif

