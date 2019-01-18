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
 
------------------------------------------------------------------------------------------ */
bool CCex::scanCommandParam(int start, int argc, char **argv)
{
	// quick exit if invalid args
	if (argc == 0 || argv == NULL) return false;


	// start checking arguments past '-command'
	std::string szCurrOpt("");
	bool bTesterFound = false;
	for (int i = start; i < argc; i++)
	{
		std::string Arg(argv[i]);

		// if previous arg is -t[ester] then we expect a param <tester>
		if (szCurrOpt.compare("tester") == 0)
		{
			if (!bTesterFound){ m_Args.set("tester", Arg); bTesterFound = true; }
			szCurrOpt.clear();
			continue;
		}

		// -t[ester] <tester> can be an argument after -command <command>. it can be in between multiple parameters of 
		// <command>. therefore it's an exception that we have to handle. note that '-t' is a special case as per unison doc, 
		// in which it always refer to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (Arg.compare("-t") == 0){ Arg = "-tester"; }
		std::vector< std::string > v;
		m_Args.get(Arg.substr(1), v);
		if (v.size() == 1)
		{
			// if tester then we expect next arg to be tester name
			if (v[0].compare("tester") == 0)
			{
				szCurrOpt = "tester"; 
				continue;	
			}
		}

		// at this point, -t[ester] is now handled if there's any. any other arg that is first is considered a <command>
		if (!m_CmdArgs.size("-command"))
		{
			// check if this command is valid. 
			if (!m_Commands.has(Arg))
			{
				
				
			}
			// invalid <command> log error
			else
			{
				m_Err.clear();
				m_Err << "CEX Error: " << Arg << ": '" << Arg << "' is not a CEX command. " << CError::endl;
				return false;
			}

			m_CmdArgs.add("-command", Arg);
			continue;
		}

		// at this point, we already got the <command> arg as well as -t[tester] <tester> if there's any.
		// next argument we might expect that requires special attention is -h[elp]. after -c <command> -h is good enough
		// and does not appear ambiguous with -head or -hd so we compare its ambiguity to a <command> argument list instead.
		m_CmdArgs.get(Arg, v);
		if (v.size() == 1)
		{
			// if it's -h[elp], we print help of the specific command
			if (v[0].compare("-help") == 0)
			{
				// if -t[ester] is found prior to -h[elp], we will connect to tester before print help <command>
				// otherwise, we will only print help <command>
				m_bConnect = bTesterFound?  true : false; 
				m_bHelp = false;
				m_bCmdHelp = true;					
				continue;	
			}
		}
		
		// if we reach this, argument is one of the parameters of -command <command>
		m_CmdArgs.add("-command", Arg);
	}

	// if there's a command, let's execute it
	if (m_CmdArgs.size("-command"))
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
 
------------------------------------------------------------------------------------------ */
bool CCex::scan(int argc, char **argv)
{ 
	// quick exit if invalid args
	if (argc == 0 || argv == NULL) return false;

	// if there's no command line args, just connect to tester with default tester name
	if (argc <= 1)
	{
		m_bConnect = true; 
		return true;
	}

	std::string szCurrOpt("");
	for (int i = 1; i < argc; i++)
	{
		std::string Arg(argv[i]);
		
		// if we're expecting opt
		if (!szCurrOpt.length())
		{
			// opts must have '-' prefix. remove it now if valid opt
			if (Arg[0] != '-')
			{
				m_Err.clear();
				m_Err << "CEX Error: CEX arguments: Bad argument to cex '" << Arg << "'" << CError::endl;
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
				m_Err.clear();
				m_Err << "CEX Error: CEX arguments: Ambiguous option '-" << Arg << "' choices are: ";
				for (unsigned int i = 0; i < v.size(); i++) m_Err << "'-" << v[i] << "', ";
				m_Err << CError::endl;
				return false;
			}
			// if this arg doesn't match any valid ones
			else if (!v.size())
			{
				m_Err.clear();
				m_Err << "CEX Error: CEX arguments: Bad argument to cex '" << Arg << "'" << CError::endl;
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
	}

	// we're done perusing the args list. are we expecting a param at this point? let's handle it
	if (szCurrOpt.length())
	{
		if (szCurrOpt.compare("tester") == 0)
		{
			m_Err.clear();
			m_Err << "CEX Error: CEX arguments: '-tester' option found but tester name missing. " << CError::endl;
			return false;
		}
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
class's constructor and destructor
------------------------------------------------------------------------------------------ */
CCex::CCex(int argc, char **argv): m_pTester(0), m_pConn(0), m_pProgCtrl(0), m_pState(0), m_pEvxio(0)
{
	// initialize flags, variables here
	m_bConnect = true; // we're connecting to tester on launch by default
	m_bHelp = false; // we're not printing any help information on launch
	m_Error.immediate = true; // hard error must be logged by default
	m_Log.enable(false); // verbose to false by default so it's not noisy	
	m_nHead = 1; // set head to default
	m_bCommand = false; // not executing command by default
	m_bCmdHelp = false; // not printing help command 

	// specify known/expected arguments. prefer to use xml file and parse it for list
	m_Args.add("command", true, "command line");
	m_Args.add("tester", true, "Specifies the target tester, if not set, the environment variable LTX_TESTER will be checked, followed by <username>_sim");
	m_Args.add("help", false, "help");
	m_Args.add("debug", false, "debug");
	m_Args.add("dm", false, "debug");
	m_Args.add("syntax_check", false, "syntax check");
	m_Args.add("timeout", true, "Time out in <seconds>");
	m_Args.add("head", true, "<head num>");
	m_Args.add("hd", true, "<head num>");
	m_Args.add("version", false, "software release version");

	m_Args.add("attempt", true, "Number of attempts to try and connect");
	m_Args.add("verbose", false, "log events in detail");

	// specify known/expected arguments for -command
	m_CmdArgs.add("-help", false, "help");
	m_CmdArgs.add("-command", false, "command");

	m_Commands.add("load", true, "load");
	m_Commands.add("get_head", true, "get_head");



	// check if we are running noisy
	m_Log.enable( m_Args.enabled("verbose") );

	// analyze command line arguments to check what user wants to do and also verify any violation
	if (!scan(argc, argv)){ m_Err.flush(); return; }

	// set default value to tester name
	if (m_Args.get("tester").empty())
	{
        	uid_t uid = getuid();
        	char buf_passw[1024];   
        	struct passwd password;
        	struct passwd *passwd_info;

        	getpwuid_r(uid, &password, buf_passw, 1024, &passwd_info);
		if (!passwd_info) m_Args.add("tester", "sim");
		else
		{
			std::stringstream ss;
			ss << passwd_info->pw_name << "_sim";
			m_Args.add("tester", ss.str());	
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
			m_Err.flush();
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

	m_Err.flush();
	disconnect();

	return;
}
 
CCex::~CCex()
{

} 

/* ------------------------------------------------------------------------------------------
print help info
------------------------------------------------------------------------------------------ */
void CCex::printHelp()
{
	m_Err.clear();
	m_Err << "LTX CEX usage:" << CError::endl;
 	m_Err << "	cex 	[-tester <tester name>] [-head(-hd) <head num>]" << CError::endl;
     	m_Err << "		[-timeout] <seconds>" << CError::endl;
     	m_Err << "		[-debug] [-dm] [-version] [-syntax_check] [-help]" << CError::endl;
     	m_Err << "		[-command <command> <arg1> <arg2> ... <argN>]" << CError::endl << CError::endl;

	m_Err << "Any unambiguous abbreviations are allowed for options to the  cex command" << CError::endl;
	m_Err << "itself as well as the commands listed below." << CError::endl;
	m_Err << "As an example you may specify: -c, -co, -com ... -command." << CError::endl;
	
	m_Err.flush();

}

/* ------------------------------------------------------------------------------------------
print help info
------------------------------------------------------------------------------------------ */
void CCex::printCmdHelp()
{
	m_Err.clear();
	m_Err << "This is Command Help for <" << m_CmdArgs.get("command") << ">" << CError::endl;
	m_Err.flush();

}

/* ------------------------------------------------------------------------------------------
execute command
------------------------------------------------------------------------------------------ */
void CCex::executeCommand()
{
	m_Err.clear();
	m_Err << "Command -- ";
	for (unsigned int i = 0; i < m_CmdArgs.size("command"); i++) m_Err << m_CmdArgs.get("command", i ) << ", ";
	m_Err << CError::endl;
	m_Err.flush();
}

/* ------------------------------------------------------------------------------------------
connects to tester by first creating EVXA tester objects and then hooking up to tester's IO
------------------------------------------------------------------------------------------ */
bool CCex::connect()
{
	// get number of attempts we'll try to connect
	long n = m_Args.getAsLong("attempt");
	if (!n) n = 1;

	std::string strTesterName = m_Args.get("tester");

	// let's attempt n number of times to connect
  	while(n--) 
	{
		disconnect();
		m_Log.print("Attempting to connect to tester <", false);
		m_Log.print(strTesterName.c_str(), false);
		m_Log.print(">...");
		// connect to tester
    		m_pTester = new TesterConnection(strTesterName.c_str());
    		if(m_pTester->getStatus() != EVXA::OK){ m_Log.print("ERROR TesterConnection constructor"); sleep(1); continue; }
		m_Log.print("TesterConnection object created...");
		
		// connect to test head
    		m_pConn = new TestheadConnection(strTesterName.c_str(), m_nHead);
    		if(m_pConn->getStatus() !=  EVXA::OK){ m_Log.print("ERROR in TestheadConnection constructor"); sleep(1); continue; }
		m_Log.print("TestheadConnection bject created...");

		// create program control object, does not check if program is loaded
    		m_pProgCtrl = new ProgramControl(*m_pConn);
    		if(m_pProgCtrl->getStatus() !=  EVXA::OK){ m_Log.print("ERROR in Program constructor"); sleep(1); continue; }
		m_Log.print("ProgramControl object created...");

		// create notification object
    		m_pState = new CStateNotification(*m_pConn);
    		if(m_pState->getStatus() !=  EVXA::OK) { m_Log.print("ERROR in statePtr constructor"); sleep(1); continue; }
		m_Log.print("CStateNotification object created...");

		// lets convert our tester name from std::string to crappy old C style string because the stupid software team 
		// who designed EVXA libraries sucks so bad and too lazy to set string arguments as constants...
		char szTesterName[1024] = "";
		sprintf(szTesterName, "%s", strTesterName.c_str());

		// create stream client
    		m_pEvxio = new CEvxioStreamClient(szTesterName, m_nHead);
    		if(m_pEvxio->getStatus() != EVXA::OK){ m_Log.print("ERROR in EvxioStreamClient constructor"); sleep(1); continue; }
		m_Log.print("CEvxioStreamClient object created...");

		// if we reached this point, we are able connect to tester. let's connect to evx stream now...
		// same issue here... i could have used stringstream but forced to use C style string because the damn EVXA class
		// wants a C style string argument that is not a constant!!!
		char szPid[1024] = "";
		sprintf(szPid, "client_%d", getpid());

    		if(m_pEvxio->ConnectEvxioStreams(m_pConn, szPid) != EVXA::OK){ m_Log.print("ERROR Connecting to evxio"); sleep(1); continue; }
    		else
		{
			// once the tester objects are created, let's wait until tester is ready
		  	while(!m_pTester->isTesterReady(m_nHead)) 
			{
				m_Log.print("Tester NOT yet ready...");
				sleep(1);
			}
			m_Log.print("Tester ready for use.");
		  	return true; 	 
		}
  	}

	// if we reach this point, we failed to connect to tester after n number of attempts...
	m_Err.clear();
	m_Err << "CEX Error: Can't connect to tester: Tester '" << strTesterName << "' does not exist." << CError::endl;
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
		handleTesterInput(strInput);
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

			handleTesterInput(buf);
		}
	}

}

/* ------------------------------------------------------------------------------------------
handle user input
------------------------------------------------------------------------------------------ */
void CCex::handleTesterInput(const std::string& strInput)
{	
	m_Log.print(strInput.c_str());
}




#endif

