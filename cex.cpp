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
	m_Log.enable = true; // verbose to false by default so it's not noisy	
	m_Log.immediate = true;

	// add main argument to cex
	m_Arg.addValid( CArg("-tester") );
	m_Arg.addValid( CArg("-help") );
	m_Arg.addValid( CArg("-command") );
	m_Arg.addValid( CArg("-head") );
	m_Arg.addValid( CArg("-hd") );
	m_Arg.addValid( CArg("-version") );
	m_Arg.addValid( CArg("-dm") );
	m_Arg.addValid( CArg("-debug") );
	m_Arg.addValid( CArg("-syntax_check") );

	// analyze command line arguments to check what user wants to do and also verify any violation
	if (!scan(argc, argv)){ m_Result.flush(); return; }


	// set default value to tester name
	if (m_Arg.get("-tester").getParam().empty())
	{
        	uid_t uid = getuid();
        	char buf_passw[1024];   
        	struct passwd password;
        	struct passwd *passwd_info;

        	getpwuid_r(uid, &password, buf_passw, 1024, &passwd_info);
		std::stringstream ss;
		if (!passwd_info) ss << "sim";
		else ss << passwd_info->pw_name << "_sim";
		m_Arg.get("-tester").addParam(ss.str());
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

	std::string szCurrOpt("");
	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
		// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (arg.compare("-t") == 0){ arg = "-tester"; }

		// get a list of matching args in the valid list of args
		std::vector< CArg > v;
		m_Arg.listValidMatch( CArg(argv[i]), v);
		
		// is it ambiguous?
		if (v.size() > 1)
		{
			m_Result.clear();
			m_Result << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
			for (unsigned int i = 0; i < v.size(); i++) m_Result << "'" << v[i].get() << "', ";
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
		if (v[0].get().compare("-tester") == 0)
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
				v[0].clearParam();
				v[0].addParam(argv[i + 1]);
				m_bConnect = true;
			}
		}

		

		// check if this is a valid

		// if -t[ester] let's handle it
		/*
		// if we're expecting opt
		if (!szCurrOpt.length())
		{
			// opts must have '-' prefix. remove it now if valid opt
			if (Arg[0] != '-')
			{
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: Bad argument to cex '" << Arg << "'" << CLog::endl;
				return false;
			}
			else Arg = Arg.substr(1);

			// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
			// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
			if (Arg.compare("t") == 0){ Arg = "tester"; }

			// if argument is an option, let's check if it's one of the valid options 
			std::vector< std::string > v;
			m_Args.get(Arg, v);
			// is this arg option ambiguous?
			if (v.size() > 1)
			{
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: Ambiguous option '-" << Arg << "' choices are: ";
				for (unsigned int i = 0; i < v.size(); i++) m_Result << "'-" << v[i] << "', ";
				m_Result << CLog::endl;
				return false;
			}
			// if this arg doesn't match any valid ones
			else if (!v.size())
			{
				m_Result.clear();
				m_Result << "CEX Error: CEX arguments: Bad argument to cex '" << Arg << "'" << CLog::endl;
				return false;
			}

			// at this point, argument is a valid and unique opt. 

			// is it help?
			if (v[0].compare("help") == 0)
			{
				m_bHelp = true;
				szCurrOpt.clear();
			}

			// is it tester?
			else if (v[0].compare("tester") == 0)
			{
				m_bConnect = true;
				szCurrOpt = v[0];
			}
			
			// is it command?
			else if (v[0].compare("command") == 0)
			{				
				return scanCommandParam(i + 1, argc, argv);
			}
		}		
		// or are we expecting param?
		else
		{
			// if this is param for -tester, we take it and immediately expect an opt. -tester only takes 1 param
			if (szCurrOpt.compare("tester") == 0)
			{
				m_Args.set("tester", Arg);
				szCurrOpt.clear();
			}
		}
*/
	}
/*
	// we're done perusing the args list. are we expecting a param at this point? let's handle it
	if (szCurrOpt.length())
	{
		if (szCurrOpt.compare("tester") == 0)
		{
			m_Result.clear();
			m_Result << "CEX Error: CEX arguments: '-tester' option found but tester name missing. " << CLog::endl;
			return false;
		}
	}
*/
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
//	m_Result << "This is Command Help for <" << m_CmdArgs.get("command") << ">" << CLog::endl;
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

	std::string strTesterName = m_Arg.get("-tester").getParam();

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

