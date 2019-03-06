#ifndef __TESTER__
#define __TESTER__

#include <evxa/ProgramControl.hxx>
#include <evxa/StateNotification.hxx>
#include <evxa/EvxioStreamsClient.hxx>
#include <utility.h>
#include <unistd.h>
#include <arg.h>

class CStateNotification: public StateNotification
{
public:
  	CStateNotification(const TestheadConnection &thc):StateNotification(thc){}
	virtual ~CStateNotification(){}
	virtual void gemStateChange(const bool linkState, const bool controlState, const bool spoolState, const char *text_msg){}
	virtual void programChange(const EVX_PROGRAM_STATE state, const char *text_msg){}
	virtual void gemTerminalMessage(const char *pcMessage){}
};


class CEvxioStreamClient: public EvxioStreamsClient
{
public:
  	CEvxioStreamClient(char *strTesterName, int nHead):EvxioStreamsClient(strTesterName, nHead) {}
	virtual ~CEvxioStreamClient(){}
};
 
class CTester: public CArg
{
private:
	// make these private to turn this class into singleton	
	CTester(); // private so we can't instantiate	
	virtual ~CTester(); // private so we can't destroy
	CTester(const CTester&){} // make copy constructor private so we can't copy                    
	const CTester& operator=(const CTester&){} // make = operator private so we can't copy

	// tester objects
  	TesterConnection *m_pTester;
  	TestheadConnection *m_pConn;
	ProgramControl *m_pProgCtrl;
  	CStateNotification *m_pState;
  	CEvxioStreamClient *m_pEvxio;
	int m_nHead;

public:
	CUtil::CLog m_Log;
	CUtil::CLog m_Debug;

public:
	static CTester& instance()
	{
		static CTester inst;
		return inst;
	}

	bool connect(const std::string& strTesterName, int nAttempts = 1);
	void disconnect();
	void loop();

	int getHead(){ return m_nHead; }
};

#endif
