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
	m_bCommand = false; // not executing command by default
	m_bCmdHelp = false; // not printing help command 

	m_Result.immediate = false; // hard error must be logged by default
	m_Result.enable = true; 
	m_Log.enable = false; // verbose to false by default so it's not noisy	
	m_Log.immediate = true;

	// add arguments valid after '-command'
	CArg* pCmd = new CArg("-command");
	pCmd->addValid( new CArg("-tester") );
	pCmd->addValid( new CArg("-help") );
	pCmd->addValid( new CArg("get_head") );
	pCmd->addValid( new CArg("get_name") );

	// add main argument to cex
	m_Arg.addValid( new CArg("-tester") );
	m_Arg.addValid( new CArg("-help") );
	m_Arg.addValid( pCmd );
	m_Arg.addValid( new CArg("-head") );
	m_Arg.addValid( new CArg("-hd") );
	m_Arg.addValid( new CArg("-version") );
	m_Arg.addValid( new CArg("-dm") );
	m_Arg.addValid( new CArg("-debug") );
	m_Arg.addValid( new CArg("-syntax_check") );

	// analyze command line arguments to check what user wants to do and also verify any violation
	if (!scan(argc, argv)){ m_Result.flush(); return; }


	// set default value to tester name
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

	// if help is enabled, print it
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
	if (m_bCommand)
	{
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
			m_Result.clear();
			m_Result << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CLog::endl;
			return false;			
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
				m_Result.clear();
				m_Result << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CLog::endl;
				return false;					
			}

			// at this point, we found a single exact match. let's handle it in the next codes
		}

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
		m_bCommand = true;

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
		m_bCommand = false;
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
	m_Result.clear();
	m_Result << "Command -- ";
//	for (unsigned int i = 0; i < m_CmdArgs.size("command"); i++) m_Result << m_CmdArgs.get("command", i ) << ", ";
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
		m_Log << "Attempting to connect to tester <" << strTesterName.c_str() << ">..." << CLog::endl;
		// connect to tester
    		m_pTester = new TesterConnection(strTesterName.c_str());
    		if(m_pTester->getStatus() != EVXA::OK){ m_Log << "ERROR TesterConnection constructor" << CLog::endl; sleep(1); continue; }
		m_Log << "TesterConnection object created..." << CLog::endl;
		
		// connect to test head
    		m_pConn = new TestheadConnection(strTesterName.c_str(), m_nHead);
    		if(m_pConn->getStatus() !=  EVXA::OK){ m_Log << "ERROR in TestheadConnection constructor" << CLog::endl; sleep(1); continue; }
		m_Log << "TestheadConnection bject created..." << CLog::endl;

		// create program control object, does not check if program is loaded
    		m_pProgCtrl = new ProgramControl(*m_pConn);
    		if(m_pProgCtrl->getStatus() !=  EVXA::OK){ m_Log << "ERROR in Program constructor" << CLog::endl; sleep(1); continue; }
		m_Log << "ProgramControl object created..." << CLog::endl;

		// create notification object
    		m_pState = new CStateNotification(*m_pConn);
    		if(m_pState->getStatus() !=  EVXA::OK) { m_Log<< "ERROR in statePtr constructor" << CLog::endl; sleep(1); continue; }
		m_Log << "CStateNotification object created..." << CLog::endl;

		// lets convert our tester name from std::string to crappy old C style string because the stupid software team 
		// who designed EVXA libraries sucks so bad and too lazy to set string arguments as constants...
		char szTesterName[1024] = "";
		sprintf(szTesterName, "%s", strTesterName.c_str());

		// create stream client
    		m_pEvxio = new CEvxioStreamClient(szTesterName, m_nHead);
    		if(m_pEvxio->getStatus() != EVXA::OK){ m_Log << "ERROR in EvxioStreamClient constructor" << CLog::endl; sleep(1); continue; }
		m_Log << "CEvxioStreamClient object created..." << CLog::endl;

		// if we reached this point, we are able connect to tester. let's connect to evx stream now...
		// same issue here... i could have used stringstream but forced to use C style string because the damn EVXA class
		// wants a C style string argument that is not a constant!!!
		char szPid[1024] = "";
		sprintf(szPid, "client_%d", getpid());

    		if(m_pEvxio->ConnectEvxioStreams(m_pConn, szPid) != EVXA::OK){ m_Log << "ERROR Connecting to evxio" << CLog::endl; sleep(1); continue; }
    		else
		{
			// once the tester objects are created, let's wait until tester is ready
		  	while(!m_pTester->isTesterReady(m_nHead)) 
			{
				m_Log << "Tester NOT yet ready..." << CLog::endl;
				sleep(1);
			}
			m_Log << "Tester ready for use." << CLog::endl;
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

}



#endif

