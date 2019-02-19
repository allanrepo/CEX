#include <cex.h>
#include <unistd.h>
#include <pwd.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/time.h>   
#include <sstream>      
#include <sys/types.h>
#include <pwd.h>

#define SAFE_DELETE(p){ delete(p); p = 0; }
 
#if 1 

void CStateNotification::gemStateChange(const bool linkState, const bool controlState, const bool spoolState, const char *text_msg)
{
	m_Log << "linkState: " << linkState << CLog::endl;
	m_Log << "controlState: " << controlState << CLog::endl;
	m_Log << "spoolState: " << spoolState << CLog::endl;
	m_Log << "Message: " << text_msg << CLog::endl;
}

void CStateNotification::programChange(const EVX_PROGRAM_STATE state, const char *text_msg) 
{
	m_Log << "Message: " << text_msg << CLog::endl;
}

void CStateNotification::gemTerminalMessage(const char *pcMessage)
{
	m_Log << "gemTerminalMessage: " << pcMessage << CLog::endl;
}

/* ------------------------------------------------------------------------------------------
class's constructor and destructor
------------------------------------------------------------------------------------------ */
CCex::CCex(int argc, char **argv): m_pTester(0), m_pConn(0), m_pProgCtrl(0), m_pState(0), m_pEvxio(0)
{
	// initialize tester flags lags
	m_nHead = 1; // set head to default

	// initialize software behavior
	m_bConnect = true; // we're connecting to tester on launch by default
	m_bHelp = false; // we're not printing any help information on launch
	m_bCmdHelp = false; // not printing help command 

	// configure loggers
	m_Result.immediate = false; // hard error must be logged by default
	m_Result.enable = true; 
	m_Log.enable = true; 	
	m_Log.immediate = true;
	m_Debug.enable = false;
	m_Debug.immediate = true;

	// create argument objects for common arguments
	CArg* pTester = new CArg("-tester");
	CArg* pHelp = new CArg("-help");

	// add arguments for -command <load>
	CArg* pLoad = new CArg("load");
	pLoad->addValid( new CArg("-display") );

	// add arguments for -command <unload>
	CArg* pUnload = new CArg("unload");
	pUnload->addValid( new CArg("-wait") );
	pUnload->addValid( new CArg("-nowait") );
	pUnload->addValid( new CArg("-dontsave") );

	// add arguments for -command <start>
	CArg* pStart = new CArg("start");
	pStart->addValid( new CArg("-ntimes") );
	pStart->addValid( new CArg("-nowait") );
	pStart->addValid( new CArg("-wait") );

	// add arguments for -command <get_exp>
	CArg* pGetExp = new CArg("get_exp");
	pGetExp->addValid( new CArg("expression") );
	pGetExp->addValid( new CArg("value") );
	pGetExp->addValid( new CArg("multi_value") );
	pGetExp->addValid( new CArg("multi_range") );

	// add arguments for -command <set_exp>
	CArg* pSetExp = new CArg("set_exp");

	// cofigure arguments for -command <evx_summary> <site>
	CArg* pSite = new CArg("site");
	pSite->addValid( new CArg("on") );
	pSite->addValid( new CArg("off") );

	// cofigure arguments for -command <evx_summary> <partial>
	CArg* pPartial = new CArg("partial");
	CArg* pFull = new CArg("full");	
	pFull->addValid( new CArg("on") );
	pFull->addValid( new CArg("off") );
	CArg* pClear = new CArg("clear");	
	pClear->addValid( new CArg("on") );
	pClear->addValid( new CArg("off") );
	pPartial->addValid( pFull );
	pPartial->addValid( pClear );
	pPartial->addValid( pSite );

	// cofigure arguments for -command <evx_summary> <final>
	CArg* pFinal = new CArg("final");
	pClear = new CArg("clear");	
	pClear->addValid( new CArg("on") );
	pClear->addValid( new CArg("off") );
	pFinal->addValid( pClear );
	pFinal->addValid( pSite );

	// configure arguments for -command <evx_summary> <final>
	CArg* pOutput = new CArg("output");
	pOutput->addValid( new CArg("lot") );
	pOutput->addValid( new CArg("sublot") );
	pOutput->addValid( new CArg("final") );
	pOutput->addValid( new CArg("partial") );

	// add arguments for -command <evx_summary>
	CArg* pSummary = new CArg("evx_summary");
	pSummary->addValid( pSite );
	pSummary->addValid( pPartial );
	pSummary->addValid( pFinal );
	pSummary->addValid( pOutput );
	pSummary->addValid( new CArg("clearpartial") );
	pSummary->addValid( new CArg("clearfinal") );
	pSummary->addValid( new CArg("details") );

	// add arguments valid after '-command'
	CArg* pCmd = new CArg("-command");
	pCmd->addValid( pTester );
	pCmd->addValid( pHelp );
	pCmd->addValid( pLoad );
	pCmd->addValid( pUnload );
	pCmd->addValid( pStart );
	pCmd->addValid( pGetExp );
	pCmd->addValid( pSetExp );
	pCmd->addValid( pSummary );
	pCmd->addValid( new CArg("get_head") );
	pCmd->addValid( new CArg("get_name") );
	pCmd->addValid( new CArg("get_username") );
	pCmd->addValid( new CArg("cex_help") );
	pCmd->addValid( new CArg("cex_version") );
	pCmd->addValid( new CArg("program_loaded") );
	pCmd->addValid( new CArg("program_load_done") );
	pCmd->addValid( new CArg("gem") );
	pCmd->addValid( new CArg("debug") );

	// add main arguments to cex
	m_Arg.addValid( pTester );
	m_Arg.addValid( pHelp );
	m_Arg.addValid( pCmd );
	m_Arg.addValid( new CArg("-head") );
	m_Arg.addValid( new CArg("-hd") );
	m_Arg.addValid( new CArg("-version") );
	m_Arg.addValid( new CArg("-dm") );
	m_Arg.addValid( new CArg("-debug") );
	m_Arg.addValid( new CArg("-syntax_check") );

	// analyze command line arguments to check what user wants to do and also verify any violation
	if (!scan(argc, argv)){ m_Result.flush(); return; }

	// set default value to tester name if -tester argument is not used
	if (m_Arg.get("-tester"))
	{
		if (m_Arg.get("-tester")->getParam().empty())
		{
        		uid_t uid = getuid();
        		char buf_passw[1024];   
        		struct passwd password;
        		struct passwd *passwd_info;
	
        		getpwuid_r(uid, &password, buf_passw, 1024, &passwd_info);
			std::stringstream ss;
			if (!passwd_info) ss << "sim";
			else ss << passwd_info->pw_name << "_sim";
			m_Arg.get("-tester")->addParam(ss.str());
		}
	}

	// if general help is enabled, print it and do nothing
	if (m_bHelp)
	{
		printHelp();
		return;
	}

	// try to connect to tester
	if (m_bConnect)
	{
		if (!connect())
		{
			m_Result.flush();
			return;
		} 
	}

	// are we executing command?
	if (m_Arg.get("-command")->getNumParam())
	{
		// either we print <help> for <command>, or execute it
		if (m_bCmdHelp){ printCmdHelp(); }
		else{ executeCommand(); }
	}
	// if not, let's get into loop
	else
	{
		loop();
	}

	m_Result.flush();
	disconnect();


	return;
}
 
CCex::~CCex()
{

} 


/* ------------------------------------------------------------------------------------------
 
------------------------------------------------------------------------------------------ */
bool CCex::scan(int argc, char **argv)
{ 
	// quick exit if invalid args
	if (argc == 0 || argv == 0) return false;

	// if there's no command line args, just connect to tester with default tester name
	if (argc <= 1)
	{
		m_bConnect = true; 
		return true;
	}

	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
		// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (arg.compare("-t") == 0) arg = "-tester"; 

		// get a list of matching args in the valid list of args
		std::vector< CArg* > v;
		m_Arg.listValidMatch( CArg(argv[i]), v);
		
		// is it ambiguous?
		if (v.size() > 1)
		{
			m_Result.clear();
			m_Result << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
			for (unsigned int i = 0; i < v.size(); i++) m_Result << "'" << v[i]->get() << "', ";
			m_Result << CLog::endl;
			return false;
		}

		// did we not find a match?
		if (!v.size())
		{
			m_Result.clear();
			m_Result << "CEX Error: CEX arguments: Bad argument to cex '" << arg << "'" << CLog::endl;
			return false;
		}

		// or did we find a match?
		
		// is it -t[ester]?
		if (v[0]->get().compare("-tester") == 0)
		{
			// expect another argument after this as <tester>
			if (i + 1 >= argc)
			{
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: -tester option found but tester name missing." << CLog::endl;
				return false;
			}
			else
			{
				v[0]->clearParam();
				v[0]->addParam(argv[++i]);
				m_bConnect = true;
			}
		}

		// is it -h[elp]?
		if (v[0]->get().compare("-help") == 0){	m_bHelp = true;	}		

		// is it -c[ommand]?
		if (v[0]->get().compare("-command") == 0)
		{
			return scanCommandParam(i + 1, argc, argv);
		}	
	}	
	
	return true;
}

/* ------------------------------------------------------------------------------------------
 
------------------------------------------------------------------------------------------ */
bool CCex::scanCommandParam(int start, int argc, char **argv)
{
	// quick exit if invalid args
	if (argc == 0 || argv == 0) return false;

	bool bTesterFound = false;
	for (int i = start; i < argc; i++)
	{
		std::string arg(argv[i]);

		// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
		// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (arg.compare("-t") == 0) arg = "-tester"; 

		// get a list of (partial) matching args in the valid list of args available on '-command'
		std::vector< CArg* > v;
		m_Arg.get("-command")->listValidMatch(CArg(argv[i]), v);
		
		// is there no match?
		if (!v.size())
		{
			// if there's no partial match and we haven't found <command> yet then this is error
			if (!m_Arg.get("-command")->getNumParam())
			{
				m_Result.clear();
				m_Result << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CLog::endl;
				return false;			
			}
			// otherwise, this can be possibly one of the parameters for <command>. we don't deal with that.
			// instead, we let <command> deal with it so we just pass it to <command> as one of its parameters
			else
			{
				m_Arg.get("-command")->get( m_Arg.get("-command")->getParam() )->addParam(argv[i]);
				continue;
			}
		}

		// is it ambiguous?
		if (v.size() > 1)
		{
			// in our first attempt to search, we did a partial match. this time, let's do an exact match
			// because this arg might be a <command>
			m_Arg.get("-command")->listValidMatch(CArg(argv[i]), v, true);

			// is it still ambiguous?
			if (v.size() > 1)
			{			
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
				for (unsigned int i = 0; i < v.size(); i++) m_Result << "'" << v[i]->get() << "', ";
				m_Result << CLog::endl;
				return false;
			}
			
			// is there no exact match?
			if (!v.size())
			{
				// if there's no partial match and we haven't found <command> yet then this is error
				if (!m_Arg.get("-command")->getNumParam())
				{
					m_Result.clear();
					m_Result << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CLog::endl;
					return false;			
				}
				// otherwise, this can be possibly one of the parameters for <command>. we don't deal with that.
				// instead, we let <command> deal with it so we just pass it to <command> as one of its parameters
				else
				{
					m_Arg.get("-command")->get( m_Arg.get("-command")->getParam() )->addParam(argv[i]);
					continue;
				}				
			}

			// at this point, we found a single exact match. let's handle it in the next codes
		}

		// at this point, we found a unique match. it can be -t[ester], -h[elp], or <command>

		// is it -t[ester]?
		if (v[0]->get().compare("-tester") == 0)
		{
			// expect another argument after this as <tester>
			if (i + 1 >= argc)
			{
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: -tester option found but tester name missing." << CLog::endl;
				return false;
			}
			else
			{
				// for some strange reason, the behavior of cex when dealing with multiple -t[ester] argument 
				// requires me to do this...
				bTesterFound = !bTesterFound;

				// increment arg index as we want to get the <tester>
				i++; 
				
				// set this <tester> to '-tester' arg object, not the arg object from '-command' 
				if (bTesterFound)
				{ 					
					m_Arg.get("-tester")->clearParam();
					m_Arg.get("-tester")->addParam(argv[i]);
					m_bConnect = m_bCmdHelp? m_bConnect : true;
				}
				continue;
			}
		}

		// is it -h[elp]?
		if (v[0]->get().compare("-help") == 0)
		{	
			//if (m_bCmdHelp) continue;

			// if -t[ester] is found prior to -h[elp], we will connect to tester before print help <command>
			// otherwise, we will only print help <command>
			m_bConnect = bTesterFound? true : false; 
			m_bHelp = false;
			m_bCmdHelp = true;					
			continue;
		}	

		// at this point, it must be the <command>
		m_Arg.get("-command")->addParam(argv[i]);		
	}

	// after scanning all args after '-command', let's check if there's a valid <command>
	if (m_Arg.get("-command")->getNumParam())
	{
		// if -help is an argument prior to -command
		if (m_bHelp)
		{
			m_bHelp = false;
			m_bCmdHelp = true;
		}
	}
	// otherwise, go loop
	else
	{
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
print help info
------------------------------------------------------------------------------------------ */
void CCex::printHelp()
{
	m_Result.clear();
	m_Result << "LTX CEX usage:" << CLog::endl;
 	m_Result << "	cex 	[-tester <tester name>] [-head(-hd) <head num>]" << CLog::endl;
     	m_Result << "		[-timeout] <seconds>" << CLog::endl;
     	m_Result << "		[-debug] [-dm] [-version] [-syntax_check] [-help]" << CLog::endl;
     	m_Result << "		[-command <command> <arg1> <arg2> ... <argN>]" << CLog::endl << CLog::endl;

	m_Result << "Any unambiguous abbreviations are allowed for options to the  cex command" << CLog::endl;
	m_Result << "itself as well as the commands listed below." << CLog::endl;
	m_Result << "As an example you may specify: -c, -co, -com ... -command." << CLog::endl;
	
	m_Result.flush();

}

/* ------------------------------------------------------------------------------------------
print help info
------------------------------------------------------------------------------------------ */
void CCex::printCmdHelp()
{
	m_Result.clear();
	m_Result << "This is Command Help for <" << m_Arg.get("-command")->getParam() << ">" << CLog::endl;
	m_Result.flush();

}

/* ------------------------------------------------------------------------------------------
execute command
------------------------------------------------------------------------------------------ */
void CCex::executeCommand()
{
	// do we actually have <command>
	if (!m_Arg.get("-command")->getNumParam())
	{
		loop();
		return;
	}

	// if we do have <command>, let's check if it's valid. if not, let's just enter loop
	CArg* pCmd = m_Arg.get("-command")->get( m_Arg.get("-command")->getParam() );
	if (!pCmd)
	{
		loop();
		return;
	} 
	
	// let's execute the appropriate function for the selected command
	std::stringstream ss;
	if ( pCmd->get().compare("get_head") == 0 ) cmdGetHead(pCmd);
	if ( pCmd->get().compare("load") == 0 ) cmdLoad(pCmd);
	if ( pCmd->get().compare("unload") == 0 ) cmdUnload(pCmd);
	if ( pCmd->get().compare("cex_version") == 0 ) cmdCexVersion(pCmd);
	if ( pCmd->get().compare("get_name") == 0 ) cmdGetName(pCmd);
	if ( pCmd->get().compare("get_username") == 0 ) cmdGetUserName(pCmd);
	if ( pCmd->get().compare("start") == 0 ) cmdStart(pCmd);
	if ( pCmd->get().compare("program_loaded") == 0 ) cmdProgramLoaded(pCmd);
	if ( pCmd->get().compare("program_load_done") == 0 ) cmdProgramLoadDone(pCmd);
	if ( pCmd->get().compare("get_exp") == 0 ) cmdGetExp(pCmd);
	if ( pCmd->get().compare("set_exp") == 0 ) cmdSetExp(pCmd);
	if ( pCmd->get().compare("evx_summary") == 0 ) cmdSummary(pCmd);
	if ( pCmd->get().compare("gem") == 0 ) cmdGem(pCmd);
	if ( pCmd->is("debug") ) cmdDebug(pCmd);

	return;

	m_Result << "Executing '" << pCmd->get() << "' command with params: ";
	//CArg* pCmd = m_Arg.get("-command")->get( m_Arg.get("-command")->getParam() );
	for (unsigned int i = 0; i < pCmd->getNumParam(); i++){ m_Result << pCmd->getParam(i) << ", "; }
	m_Result << CLog::endl;		
	m_Result.flush();
}

/* ------------------------------------------------------------------------------------------
connects to tester by first creating EVXA tester objects and then hooking up to tester's IO
------------------------------------------------------------------------------------------ */
bool CCex::connect()
{
	// get number of attempts we'll try to connect
	long n = 1;
	if (!n) n = 1;

	std::string strTesterName = m_Arg.get("-tester")? m_Arg.get("-tester")->getParam() : "";

	// let's attempt n number of times to connect
  	while(n--) 
	{
		disconnect();
		m_Debug << "Attempting to connect to tester <" << strTesterName.c_str() << ">..." << CLog::endl;
		// connect to tester
    		m_pTester = new TesterConnection(strTesterName.c_str());
    		if(m_pTester->getStatus() != EVXA::OK){ m_Debug << "ERROR TesterConnection constructor" << CLog::endl; /*sleep(1)*/; continue; }
		m_Debug << "TesterConnection object created..." << CLog::endl;
		
		// connect to test head
    		m_pConn = new TestheadConnection(strTesterName.c_str(), m_nHead);
    		if(m_pConn->getStatus() !=  EVXA::OK){ m_Debug << "ERROR in TestheadConnection constructor" << CLog::endl; /*sleep(1)*/; continue; }
		m_Debug << "TestheadConnection bject created..." << CLog::endl;

		// create program control object, does not check if program is loaded
    		m_pProgCtrl = new ProgramControl(*m_pConn);
    		if(m_pProgCtrl->getStatus() !=  EVXA::OK){ m_Debug << "ERROR in Program constructor" << CLog::endl; /*sleep(1)*/; continue; }
		m_Debug << "ProgramControl object created..." << CLog::endl;

		// create notification object
    		m_pState = new CStateNotification(*m_pConn);
    		if(m_pState->getStatus() !=  EVXA::OK) { m_Debug<< "ERROR in statePtr constructor" << CLog::endl; /*sleep(1)*/; continue; }
		m_Debug << "CStateNotification object created..." << CLog::endl;

		// lets convert our tester name from std::string to crappy old C style string because the stupid software team 
		// who designed EVXA libraries sucks so bad and too lazy to set string arguments as constants...
		char szTesterName[1024] = "";
		sprintf(szTesterName, "%s", strTesterName.c_str());

		// create stream client
    		m_pEvxio = new CEvxioStreamClient(szTesterName, m_nHead);
    		if(m_pEvxio->getStatus() != EVXA::OK){ m_Debug << "ERROR in EvxioStreamClient constructor" << CLog::endl; /*sleep(1)*/; continue; }
		m_Debug << "CEvxioStreamClient object created..." << CLog::endl;

		// if we reached this point, we are able connect to tester. let's connect to evx stream now...
		// same issue here... i could have used stringstream but forced to use C style string because the damn EVXA class
		// wants a C style string argument that is not a constant!!!
		char szPid[1024] = "";
		sprintf(szPid, "client_%d", getpid());

    		if(m_pEvxio->ConnectEvxioStreams(m_pConn, szPid) != EVXA::OK){ m_Debug << "ERROR Connecting to evxio" << CLog::endl; /*sleep(1)*/; continue; }
    		else
		{
			// once the tester objects are created, let's wait until tester is ready
		  	while(!m_pTester->isTesterReady(m_nHead)) 
			{
				m_Debug << "Tester NOT yet ready..." << CLog::endl;
				/*sleep(1)*/;
			}
			m_Debug << "Tester ready for use." << CLog::endl;
		  	return true; 	 
		}
  	}

	// if we reach this point, we failed to connect to tester after n number of attempts...
	m_Result.clear();
	m_Result << "CEX Error: Can't connect to tester: Tester '" << strTesterName << "' does not exist." << CLog::endl;
  	return false; 	 
}

/* ------------------------------------------------------------------------------------------
destroys the EVXA tester objects
------------------------------------------------------------------------------------------ */
void CCex::disconnect()
{
	SAFE_DELETE( m_pTester );
	SAFE_DELETE( m_pConn );
	SAFE_DELETE( m_pProgCtrl );
	SAFE_DELETE( m_pState );
	SAFE_DELETE( m_pEvxio );
}

/* ------------------------------------------------------------------------------------------
application's loop
------------------------------------------------------------------------------------------ */
void CCex::loop()
{
/*
	while(1)
	{
		std::string strInput;
		std::cout << "cex>";
		std::getline( std::cin, strInput); 
	}

	while(0) 
	{
		fd_set readfd;
		FD_ZERO(&readfd);
 		FD_SET(fileno(stdin), &readfd); //add input to select

		if((select(1, &readfd, NULL, NULL, NULL)) < 0) {  }

		if(FD_ISSET(fileno(stdin), &readfd))
		{

 			char buf[BUFSIZ] = "";
  			read(fileno(stdin), buf, BUFSIZ);

		}
	}
*/
	while(true)
	{
		fd_set read_fd;
		int stateSockId = m_pState->getSocketId();
		int evxioSockId = m_pEvxio->getEvxioSocketId();
		int errorSockId = m_pEvxio->getErrorSocketId();

		FD_ZERO(&read_fd);
		FD_SET(fileno(stdin), &read_fd); //add input to select
		FD_SET(stateSockId, &read_fd); //add state notifications to select
		FD_SET(evxioSockId, &read_fd); // add evxio notifications to select
		FD_SET(errorSockId, &read_fd); // add error notifications to select

		int num_fds = ((stateSockId > evxioSockId) ? stateSockId : evxioSockId);
		num_fds = ((num_fds > errorSockId) ? num_fds + 1 : errorSockId +1);

		int num_ready;
		if((num_ready = select(num_fds, &read_fd, NULL, NULL, NULL)) < 0) 
		{
			perror("main_serverLoop select failed ");
		}

		if(FD_ISSET(fileno(stdin), &read_fd)) // handle requests from stdin
		{
			char buf[1024] = "";
			read(fileno(stdin), buf, 1024);
			fprintf(stdout, ">> %s\n", buf);
		}


		if((stateSockId > 0) && (FD_ISSET(stateSockId, &read_fd))) 
		{//handle requests for state notifications.
			if(m_pState->respond(stateSockId) != EVXA::OK) 
			{
				const char *errbuf = m_pState->getStatusBuffer();
				fprintf(stdout, "ERROR state respond: %s\n", errbuf);
				return ;
			}  
		}
		if((evxioSockId > 0) && (FD_ISSET(evxioSockId, &read_fd))) 
		{//handle requests for evxio notifications.
			if(m_pEvxio->streamsRespond() != EVXA::OK) 
			{
		//    const char *errbuf = TesterObjects.evxioPtr->getStatusBuffer();
		//    fprintf(stdout, "ERROR stream respond: %s\n", errbuf);
		//     testerReconnect = 1;
				return ;
			}
		}
		if((errorSockId > 0) && (FD_ISSET(errorSockId, &read_fd))) 
		{//handle requests for evxio notifications.
			if(m_pEvxio->ErrorRespond() != EVXA::OK) 
			{
		//     const char *errbuf = TesterObjects.evxioPtr->getStatusBuffer();
		//     fprintf(stdout, "ERROR error respond: %s\n", errbuf);
		//     testerReconnect = 1;
			return ;
			}
		}
	}
}

/* ------------------------------------------------------------------------------------------
handle get_head command
------------------------------------------------------------------------------------------ */
bool CCex::cmdGetHead(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Unknown option '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	m_Log << "CEX: Head number " << m_nHead << CLog::endl;
	return true;
}

/* ------------------------------------------------------------------------------------------
handle load command
------------------------------------------------------------------------------------------ */
bool CCex::cmdLoad(const CArg* pCmd)
{
	if (!pCmd) return false; 

	std::vector< std::string > v;
	bool bDisplay = false;
	for (unsigned int i = 0; i < pCmd->getNumParam(); i++)
	{
		// is param not a valid arg? then it might be the test program path to load
		if (!pCmd->get( pCmd->getParam(i), true ))
		{
			v.push_back( pCmd->getParam(i) );
		}
		// or it can be a valid arg. valid arg must be exact match
		else
		{
			if (pCmd->get( pCmd->getParam(i), true )->get().compare("-display") == 0) bDisplay = true;
		}		
	}	

	// if multiple programs are specified
	if (v.size() > 1)
	{
		m_Result << "CEX Error: load: Multiple program names found, ";
		for (unsigned int i = 0; i < v.size(); i++){ m_Result << "'" << v[i] << "', "; }
		m_Result << CLog::endl;
		return false;
	}
	// if no program is specified
	else if (!v.size())
	{
		m_Result << "CEX Error: load: Missing test program name (ltx/cex)" << CLog::endl;
		return false;
	}
	// let's load program
	else
	{
		m_Log << "CEX: Program " << v[0] << " is loading " << (bDisplay? "with" : "without") << " display..." << CLog::endl;
		m_pProgCtrl->load( v[0].c_str(), EVXA::WAIT, bDisplay? EVXA::DISPLAY : EVXA::NO_DISPLAY );
		if ( m_pProgCtrl->getStatus() != EVXA::OK )
		{
			m_Result << "CEX Error: Error in loading " << v[0] << CLog::endl;
			return false;
		}
		if ( m_pProgCtrl->isProgramLoaded() )
		{
			m_Log << "CEX: Loaded program " << m_pProgCtrl->getProgramName() << "." << CLog::endl;
			return true;
		}
		else
		{
			m_Result << "CEX Error: There is no program loaded." << CLog::endl;
			return false;
		}
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
handle unload command
------------------------------------------------------------------------------------------ */
bool CCex::cmdUnload(const CArg* pCmd)
{
	if (!pCmd) return false; 
	
	bool bSave = true; // option to allow user to save program before unloading
	long nWait = 0;	 // 0 by default. if -wait 0, we wait forever
	bool bWait = true; // -wait or -nowait
	
	// let's find any invalid arg
	std::vector< std::string > v;
	for (unsigned int i = 0; i < pCmd->getNumParam(); i++)
	{
		// is param not valid? error then...
		if (!pCmd->get( pCmd->getParam(i), true ))
		{
			v.push_back( pCmd->getParam(i) );
		}

		// or it can be a valid arg. valid arg must be exact match
		else
		{
			// if -wait is found, let's take the next param as the <wait> value.
			if (pCmd->getParam(i).compare("-wait") == 0)
			{
				// is there no more argument after '-wait'?
				if (i + 1 >= pCmd->getNumParam())
				{
					m_Result << "CEX Error: unload: 'end of line' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}
				// is the argument after '-wait' a number?
				if ( !isInteger( pCmd->getParam(i + 1) ) )
				{
					m_Result << "CEX Error: unload: '" << pCmd->getParam(i + 1) << "' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}		
				// let's get the number
				nWait = toLong( pCmd->getParam(++i) );										
			}
			
			// found '-nowait' param
			if (pCmd->getParam(i).compare("-nowait") == 0)
			{
				// if '-wait' is also a param then it's an error
				if (pCmd->hasParam("-wait"))
				{
					m_Result << "CEX Error: unload: No-wait with wait interval not available." << CLog::endl;
					return false;
				}
				bWait = false;							
			}
			// found -dontsave param
			if (pCmd->getParam(i).compare("-dontsave") == 0)
			{
				bSave = false;
			}
		}		
	}

	// did we find invalid args?
	if (v.size())
	{
		m_Result << "CEX Error: unload: Unknown parameter '" << v[0] << "'." << CLog::endl;
		return false;
	}	

	// unload the program
	if ( !m_pProgCtrl->isProgramLoaded() )
	{
		m_Result << "CEX Error: There is no program loaded." << CLog::endl;
		return false;
	}

	const char* szProgramName =  m_pProgCtrl->getProgramName();
	m_Log << "CEX: Program " << szProgramName << " is unloading. This may take a few moments...." << CLog::endl;

	// execute evxa command to unload program
	// if nWait = 0, it will wait forever
	// bSave is ignored by evxa and original CEX command so must figure a work around to implement this behavior
	m_pProgCtrl->unload( bWait? EVXA::WAIT : EVXA::NO_WAIT, nWait, !bSave );

	if ( m_pProgCtrl->getStatus() != EVXA::OK )
	{
		m_Result << "CEX Error: Error in unloading " << szProgramName << CLog::endl;
		return false;
	}

	if ( !m_pProgCtrl->isProgramLoaded() || !bWait)
	{
		m_Log << "CEX: Unloaded program " << szProgramName << "." << CLog::endl;
		return true;
	}
	else
	{
		m_Result << "CEX Error: Program " << m_pProgCtrl->getProgramName() << " is still loaded." << CLog::endl;
		return false;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
handle cex_version command
------------------------------------------------------------------------------------------ */
bool CCex::cmdCexVersion(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: cex_version: does not accept parameters. Found '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	m_Result.clear();
	m_Result << "CEX Apps Version Developed by Allan Asis. Rev 0.1" << CLog::endl;
	m_Result.flush();
	return true;
}

/* ------------------------------------------------------------------------------------------
handle get_name command
------------------------------------------------------------------------------------------ */
bool CCex::cmdGetName(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: get_name: Unknown parameter '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	m_Result.clear();
	m_Result << "CEX: Name of Tester : " << m_pTester->getName() << CLog::endl;
	m_Result.flush();
	return true;
}


/* ------------------------------------------------------------------------------------------
handle get_user_name command
------------------------------------------------------------------------------------------ */
bool CCex::cmdGetUserName(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: get_name: Unknown parameter '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	m_Result.clear();
	m_Result << "CEX: Current session owner: " << m_pProgCtrl->getUserName() << CLog::endl;
	m_Result.flush();
	return true;
}

/* ------------------------------------------------------------------------------------------
handle start command
- 	default is -wait <0>
- 	default is execute start only once and will not log loop count message
------------------------------------------------------------------------------------------ */
bool CCex::cmdStart(const CArg* pCmd)
{
	if (!pCmd) return false; 

	// defaults
	bool bLoop = false; // no loop
	int nLoop = 1; // execute once	
	int nWaitAfterExec = 0; // wait time in <sec> after each execution
	bool bExitAfterExec = false; // -nowait flag; 
	bool bWaitAfterExec = false; // -wait flag

	// let's find any invalid arg
	std::vector< std::string > v;
	for (unsigned int i = 0; i < pCmd->getNumParam(); i++)
	{
		// is param not valid? error then...
		if (!pCmd->get( pCmd->getParam(i), true ))
		{
			v.push_back( pCmd->getParam(i) );
		}
		// or it can be a valid arg. valid arg must be exact match
		else
		{
			// if -wait is found, let's take the next param as the <wait> value.
			if (pCmd->getParam(i).compare("-wait") == 0)
			{
				// is there no more argument after '-wait'?
				if (i + 1 >= pCmd->getNumParam())
				{
					m_Result << "CEX Error: start: 'end of line' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}

				// is the argument after '-wait' a number?
				if ( !isInteger( pCmd->getParam(i + 1) ) )
				{
					m_Result << "CEX Error: start: '" << pCmd->getParam(i + 1) << "' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}

				// let's now disable  -nowait flag if it was set prior to this
				bExitAfterExec = false;

				// let's also get the number as wait in <sec>, ensure it's min as 0
				nWaitAfterExec = toLong( pCmd->getParam(++i) );	
				nWaitAfterExec = nWaitAfterExec < 0 ? 0 : nWaitAfterExec;

				// enable wait after execution
				bWaitAfterExec = true;									
			}
			
			// found '-nowait' param
			if (pCmd->getParam(i).compare("-nowait") == 0)
			{
				// if -wait <sec> is already found prior to this,it's error
				if ( bWaitAfterExec )
				{
					m_Result << "CEX Error: unload: No-wait with wait interval not available." << CLog::endl;
					return false;
				}
				// otherwise, let's do a -nowait execution
				bExitAfterExec = true;							
			}

			// found -ntimes param
			if (pCmd->getParam(i).compare("-ntimes") == 0)
			{
				// is there no more argument after '-ntimes'?
				if (i + 1 >= pCmd->getNumParam())
				{
					m_Result << "CEX Error: start: 'end of line' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}

				// is the argument after '-ntimes' a number?
				if ( !isInteger( pCmd->getParam(i + 1) ) )
				{
					m_Result << "CEX Error: start: '" << pCmd->getParam(i + 1) << "' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}

				// let's now disable -nowait flag if it was set prior to this
				bExitAfterExec = false;

				// let's set loop to true 
				bLoop = true;

				// let's also get the number as number of loops, ensure it's min as 1
				nLoop = toLong( pCmd->getParam(++i) );	
				nLoop = nLoop < 1 ? 1 : nLoop;				
			}
		}		
	}

	// did we find invalid args?
	if (v.size())
	{
		m_Result << "CEX Error: unload: Unknown parameter '" << v[0] << "'." << CLog::endl;
		return false;
	}	

	// execute the program now
	for (int i = 0; i < nLoop; i++)
	{
		if (bLoop) m_Log << "Looping (" << i + 1 << "/" << nLoop << ")" << CLog::endl; // replicate original CEX printing this on loop

		m_pProgCtrl->start( bExitAfterExec? EVXA::NO_WAIT : EVXA::WAIT );

		// if -nowait, we immediately exit after executing
		if (bExitAfterExec) return true;
		
		// -wait <sec> occurs AFTER every execution
		if (nWaitAfterExec) sleep(nWaitAfterExec);
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
handle program_loaded command
------------------------------------------------------------------------------------------ */
bool CCex::cmdProgramLoaded(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Unknown option '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	if (m_pProgCtrl->isProgramLoaded()) m_Log << "CEX: Program " << m_pProgCtrl->getProgramName() << " is currently loaded." << CLog::endl;
	else m_Log << "CEX: There is currently no program loaded." << CLog::endl;
	return true;
}

/* ------------------------------------------------------------------------------------------
handle program_load_done command
------------------------------------------------------------------------------------------ */
bool CCex::cmdProgramLoadDone(const CArg* pCmd)
{
	if (!pCmd) return false; 

	if (pCmd->getNumParam())
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Unknown option '" << pCmd->getParam() << "'." << CLog::endl;
		return false;
	}

	if (m_pProgCtrl->isProgramLoadDone()) m_Log << "CEX: Program " << m_pProgCtrl->getProgramName() << " is done loading." << CLog::endl;
	else m_Log << "CCEX: Program is not done loading." << CLog::endl;
	return true;
}

/* ------------------------------------------------------------------------------------------
handle get_exp command
------------------------------------------------------------------------------------------ */
bool CCex::cmdGetExp(const CArg* pCmd)
{
	if (!pCmd) return false; 

	// if there's no param after this command then it's error. we're expecting <expression> to immediately follow -c <get_exp>
	if (!pCmd->getNumParam())
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Missing expression name." << CLog::endl;
		return false;
	}

	// if there's no param  after <expression> then it's error. we're expecting one of the -display options
	if (pCmd->getNumParam() < 2)
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Missing mode name." << CLog::endl;
		return false;
	}

	// we strictly expect only 2 arguments after this command - <expression> and -display. anything else is error.
	if (pCmd->getNumParam() > 2)
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Multiple mode names found - ";
		for (unsigned int i = 1; i < pCmd->getNumParam(); i++)
		{
			m_Result << "'" << pCmd->getParam(i) << "', ";
		}
		m_Result << CLog::endl;
		return false;
	}

	// check if mode name is valid
	CArg* pMode = pCmd->get( pCmd->getParam(1), true );
	if (!pMode)
	{
		m_Result << "CEX Error: Unknown display mode type. " << pCmd->getParam(1) <<  CLog::endl;
		return false;
	}
	else
	{
		EVX_EXPR_DISPLAY_MODE nDisplayMode;
		if ( pCmd->getParam(1).compare("expression") == 0 ) nDisplayMode = EVX_SHOW_EXPRESSION;
		if ( pCmd->getParam(1).compare("value") == 0 ) nDisplayMode = EVX_SHOW_VALUE;
		if ( pCmd->getParam(1).compare("multi_value") == 0 ) nDisplayMode = EVX_SHOW_MULTI_VALUE;
		if ( pCmd->getParam(1).compare("multi_range") == 0 ) nDisplayMode = EVX_SHOW_MULTI_RANGE;
		m_Log << m_pProgCtrl->getExpression( pCmd->getParam(0).c_str(), nDisplayMode) << CLog::endl;
	}
	return true;
}


/* ------------------------------------------------------------------------------------------
handle set_exp command
------------------------------------------------------------------------------------------ */
bool CCex::cmdSetExp(const CArg* pCmd)
{
	return true;
}


/* ------------------------------------------------------------------------------------------
handle evx_summary command
-	it's default response is to print out unison summary states
-	this command has several option:
	-	details, site, partial, final, output, type
	- 	the first arg after evx_summary must be one of the above
	-	succeeding arg is an option and the next one after that (if exists)
		is either param for the previous option or another option
	-	e.g. evx_summary <partial <clear> <full> <on> 
		-	<partial> is evx_summary option
		-	<clear> is one of <partial>'s option
		-	<full> is one of <partial>'s option
		-	<on> is <full>'s option
	-	some options must have option such as <partial> 
		while others don't e.g. <site>	

------------------------------------------------------------------------------------------ */
bool CCex::cmdSummary(const CArg* pCmd)
{
	if (!pCmd) return false; 

	// if there's no options, then it's just a query
	if (!pCmd->getNumParam())
	{
		m_Result << "evx_summary status:" << CLog::endl;

		// print site state
		EVXA::ON_OFF_TYPE state = m_pProgCtrl->getSummary(EVX_UpdateBreakout);
		m_Result << "    site     " << (state == EVXA::ON? "on" : "off") << CLog::endl;

		// print lot type
		EVX_LOT_TYPE_SUMMARY lot = m_pProgCtrl->getLotTypeSummary();
		m_Result << "    lot_type " << (lot == EVX_SUBLOT_SUMMARY? "sublot" : "lot") << CLog::endl;

		// print partial status
		m_Result << "    partial  full " << (m_pProgCtrl->getSummary(EVX_UpdateFinal) == EVXA::ON? "on" : "off");
		m_Result << ",  clear " << (m_pProgCtrl->getSummary(EVX_ClearPartial) == EVXA::ON? "on" : "off") <<  CLog::endl;

		// print final status
		m_Result << "    final    clear " << (m_pProgCtrl->getSummary(EVX_ClearFinal) == EVXA::ON? "on" : "off") << CLog::endl;
		return true;
	}

	// the first option must be valid and will be considered as primary option. succeeding ones will be parameters of this primary option
	CArg* pSummaryType = pCmd->get( pCmd->getParam(0), true );
	if (!pSummaryType)
	{
		m_Result << "CEX Error: evx_summary: " << pCmd->getParam(0) << " is not a valid option." << CLog::endl;
		return false;
	}

	// analyze succeeding args if any
	for (unsigned int i = 1; i < pCmd->getNumParam(); i++)
	{
		// search our summary type if this param is one of its valid options. add it in its list
		if (pSummaryType->get( pCmd->getParam(i), true ))
		{
			pSummaryType->addParam( pCmd->getParam(i) );
		}
		// if this arg is not a valid option for our current summary type, it might be a option for the current summary type's latest option
		else
		{
			// let's check first if current summary type even has options
			if (pSummaryType->getNumParam())
			{					
				// try to get the summary option from current summary type's latest option 
				CArg* pSummaryOption = pSummaryType->get( pSummaryType->getParam(pSummaryType->getNumParam() - 1), true );
				if (pSummaryOption) 
				{
					// if summary option matches (e.g. on, off, etc...), we set it
					if (pSummaryOption->get( pCmd->getParam(i), true )) 
					{
						pSummaryOption->addParam( pCmd->getParam(i) );
						continue;
					}
				}
			}
			// if we reached this point, either this arg is not a valid option for current summary type's latest option or current 
			// summary type doesn't even have an option at all. it might be an ERROR for certain summary options but we don't handle it here. 
			// instead we handle when we're about to execute command so we just pass it as summary type's option. that's how original CEX behaves
			pSummaryType->addParam( pCmd->getParam(i) ); 
		
		}
	}

	// if summary type is <site>
	if (pSummaryType->is("site", true))
	{
		// do the job. note that we only care about first option of <site>
		if ( pSummaryType->isParam("on") ) m_pProgCtrl->setSummary(EVX_UpdateBreakout, EVXA::ON);		
		else if ( pSummaryType->isParam("off") ) m_pProgCtrl->setSummary(EVX_UpdateBreakout, EVXA::OFF);
		else m_pProgCtrl->setSummary(EVX_UpdateBreakout, m_pProgCtrl->getSummary(EVX_UpdateBreakout) == EVXA::ON? EVXA::OFF : EVXA::ON);

		// display results 
		m_Result << "CEX: evx_summary ";
		m_Result << pSummaryType->get();
		m_Result << " option has been " << ( (pSummaryType->isParam("on") || pSummaryType->isParam("off"))? "set":"toggled") << " to ";
		m_Result << (m_pProgCtrl->getSummary(EVX_UpdateBreakout) == EVXA::ON? "ON" : "OFF") << "." << CLog::endl;
	}
	// if summary type is <clearfinal> or <clearpartial>
	else if ( pSummaryType->get().compare("clearfinal") == 0 || pSummaryType->get().compare("clearpartial") == 0)
	{
		if (pSummaryType->get().compare("clearfinal") == 0) m_pProgCtrl->clearFinalSummary();
		else m_pProgCtrl->clearPartialSummary();
		m_Result << "CEX: cleared "<< (pSummaryType->get().compare("clearfinal") == 0? "final":"partial") << " summary." << CLog::endl;
	}
	// if summary type is <output>
	else if ( pSummaryType->is("output", true) )
	{		
		bool bFinal = m_pProgCtrl->getSummary(EVX_ClearFinal) == EVXA::ON? true:false;
		for (unsigned int i = 0; i < pSummaryType->getNumParam(); i++)
		{
			// get the summary option for this param
			CArg* pSummaryOption = pSummaryType->get( pSummaryType->getParam(i), true );		
			if (!pSummaryOption)
			{
				m_Result << "CEX Error: evx_summary: " << pSummaryType->getParam(i)  << " is not a valid " << pSummaryType->get() << " summary option." << CLog::endl;
				return false;
			}		 
			// we set lot/sublot option here because in CEX, the last one gets the dibs
			if (pSummaryOption->is("lot")) m_pProgCtrl->setLotTypeSummary(EVX_LOT_SUMMARY);
			if (pSummaryOption->is("sublot")) m_pProgCtrl->setLotTypeSummary(EVX_SUBLOT_SUMMARY);
			if (pSummaryOption->is("final")) bFinal = true; 
			if (pSummaryOption->is("partial")) bFinal = false; 
		}
		// do the job
		bFinal? m_pProgCtrl->outputFinalSummary() : m_pProgCtrl->outputPartialSummary();

		// display results
		m_Result << CLog::endl << "CEX: evx_summary output -- " << (bFinal? "Final" : "Partial") << "/" << (m_pProgCtrl->getLotTypeSummary() == EVX_LOT_SUMMARY? "Lot":"Sublot") << CLog::endl;
		if (bFinal)
		{
			m_Result << "     Clearing results: Sublot" << (m_pProgCtrl->getLotTypeSummary() == EVX_LOT_SUMMARY? ", Lot":"") << CLog::endl;
			m_Result << "     Reseting NextSerial to '1'." << CLog::endl << CLog::endl;
		}
	}
	// if summary type is <partial> or <final>
	else
	{
		// if there's no param, it's ERROR
		if (!pSummaryType->getNumParam())
		{
			m_Result << "CEX Error: evx_summary: Missing argument to the " << pSummaryType->get() << " option." << CLog::endl;
			return false;		
		}
		for (unsigned int i = 0; i < pSummaryType->getNumParam(); i++)
		{
			// get the summary option for this param
			CArg* pSummaryOption = pSummaryType->get( pSummaryType->getParam(i), true );		
	
			if (!pSummaryOption)
			{
				m_Result << "CEX Error: evx_summary: " << pSummaryType->getParam(i)  << " is not a valid " << pSummaryType->get() << " summary option." << CLog::endl;
				return false;
			}

			// let's specify which summary type are we going to process			
			EVX_SUMMARY_TYPE st;
			if ( pSummaryType->get().compare("partial") == 0 && pSummaryOption->get().compare("full") == 0) st = EVX_UpdateFinal; // UpdateFinal - partial full
			else if ( pSummaryType->get().compare("partial") == 0 && pSummaryOption->get().compare("clear") == 0) st = EVX_ClearPartial; // ClearPartial - partial clear
			else if ( pSummaryType->get().compare("final") == 0 && pSummaryOption->get().compare("clear") == 0) st = EVX_ClearFinal; // ClearFinal - final clear
			else if ( pSummaryOption->get().compare("site") == 0 ) st = EVX_UpdateBreakout; // UpdateBreakout - site
			else continue;
			
			// do the job
			if ( pSummaryOption->hasParam("on") ) m_pProgCtrl->setSummary(st, EVXA::ON);
			else if ( pSummaryOption->hasParam("off") ) m_pProgCtrl->setSummary(st, EVXA::OFF);
			else m_pProgCtrl->setSummary(st, m_pProgCtrl->getSummary(st) == EVXA::ON? EVXA::OFF : EVXA::ON);

			// display results 
			m_Result << "CEX: evx_summary ";
			m_Result << (pSummaryOption->get().compare("site") == 0? "": pSummaryType->get());
			m_Result << (pSummaryOption->get().compare("site") == 0? "": " ") << pSummaryOption->get();
			m_Result << " option has been " << ( (pSummaryOption->hasParam("on") || pSummaryOption->hasParam("off"))? "set":"toggled") << " to ";
			m_Result << (m_pProgCtrl->getSummary(st) == EVXA::ON? "ON" : "OFF") << "." << CLog::endl;
		}
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
handle gem command
------------------------------------------------------------------------------------------ */
bool CCex::cmdGem(const CArg* pCmd)
{
	if (!pCmd) return false; 

	// if no command
	if (!pCmd->getNumParam())
	{
		m_Result << "CEX Error: " << pCmd->get() << ": Missing expression name." << CLog::endl;
		return false;
	}
	m_Result << "GEM command: " << pCmd->getParam(0) << CLog::endl;
	
	m_pProgCtrl->gemSendMsgToHost(pCmd->getParam(0));
	return true;
}

/* ------------------------------------------------------------------------------------------
use this for testing stuff...
------------------------------------------------------------------------------------------ */
bool CCex::cmdDebug(const CArg* pCmd)
{
	if (!pCmd) return false; 

	m_Log << "Num Datalogs: " << m_pProgCtrl->getNumDatalogs() << CLog::endl;
	for (int i = 0; i < m_pProgCtrl->getNumDatalogs(); i++)
	{
		m_Log << "    [" << i << "] Attributes: " << m_pProgCtrl->getNumDatalogAttributes(i) << CLog::endl;
		for (int j = 0; j < m_pProgCtrl->getNumDatalogAttributes(i); j++)
		{
			m_Log << "        [" << j << "] " << m_pProgCtrl->getDlogAttributeString(i, j) << CLog::endl;
		}
	}
	m_Log << CLog::endl << "Num Datalogs: " << m_pProgCtrl->getNumDatalogs() << CLog::endl;
//	for (int i = 0; i < m_pProgCtrl->getNumDatalogs(); i++)
	for (int i = 0; i < 5; i++)
	{
		
		//m_Log << "    [" << i << "] " << m_pProgCtrl->getDatalogFormat(i) << CLog::endl;
		m_Log << "----[" << i  << "] " << m_pProgCtrl->getDatalogString(i, -1, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getDatalogString(i, 0, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getDatalogString(i, 1, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getDatalogString(i, 2, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getDatalogString(i, 3, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getDatalogString(i, 4, 0) << CLog::endl;
		m_Log << "    [" << i  << "] " << m_pProgCtrl->getTestIdForStream(i, 1) << CLog::endl;

//		int nDlogFormat = m_pProgCtrl->getNumDatalogFormats();

	}

	return true;
}

#endif

