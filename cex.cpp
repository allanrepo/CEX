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

	// add arguments valid after '-command'
	CArg* pCmd = new CArg("-command");
	pCmd->addValid( pTester );
	pCmd->addValid( pHelp );
	pCmd->addValid( pLoad );
	pCmd->addValid( pUnload );
	pCmd->addValid( pStart );
	pCmd->addValid( new CArg("get_head") );
	pCmd->addValid( new CArg("get_name") );
	pCmd->addValid( new CArg("get_username") );
	pCmd->addValid( new CArg("cex_help") );
	pCmd->addValid( new CArg("cex_version") );

	// add main argument to cex
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
				m_Result.clear();
				m_Result << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CLog::endl;
				return false;					
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

	// if we do have <command>, let's check if it's valid
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
------------------------------------------------------------------------------------------ */
bool CCex::cmdStart(const CArg* pCmd)
{/*
	if (!pCmd) return false; 

	int nWait = 0;
	bool bWait = true; 

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

				// if '-nowait' is also a param then it's an error
				if (pCmd->hasParam("-nowait"))
				{
					m_Result << "CEX Error: unload: No-wait with wait interval not available." << CLog::endl;
					return false;
				}

				// is the argument after '-wait' a number?
				if ( !isInteger( pCmd->getParam(i + 1) ) )
				{
					m_Result << "CEX Error: start: '" << pCmd->getParam(i + 1) << "' found where 'integer' expected (ltx/tkn)" << CLog::endl;
					return false;
				}		
				// let's get the number
				nWait = toLong( pCmd->getParam(++i) );	

				// enable wait
				bWait = true;									
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
*/
	return true;
}





#endif

