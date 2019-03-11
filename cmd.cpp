#include <cmd.h>
#include <utility.h>

/* ------------------------------------------------------------------------------------------
prints out general help
------------------------------------------------------------------------------------------ */
bool CHelp::exec()
{
	m_Log << "" << CUtil::CLog::endl;
	m_Log << "LTX CEX usage: " << CUtil::CLog::endl; 
	m_Log << "cex [-tester <tester name>] [-head(-hd) <head num>]" << CUtil::CLog::endl;
	m_Log << "     [-timeout] <seconds>" << CUtil::CLog::endl; 
	m_Log << "     [-debug] [-dm] [-version] [-syntax_check] [-help]" << CUtil::CLog::endl;
	m_Log << "     [-command <command> <arg1> <arg2> ... <argN>]" << CUtil::CLog::endl;
	m_Log << "" << CUtil::CLog::endl;
	m_Log << "Any unambiguous abbreviations are allowed for options to the  cex command" << CUtil::CLog::endl;
	m_Log << "itself as well as the commands listed below" << CUtil::CLog::endl;
	m_Log << "As an example you may specify: -c, -co, -com ... -command." << CUtil::CLog::endl;
	m_Log << "" << CUtil::CLog::endl;
	m_Log << "The following environment variables are recognized to specify" << CUtil::CLog::endl;
	m_Log << "the tester environment:                  " << CUtil::CLog::endl;
	m_Log << "    LTX_TESTER=<tester name>   - specify tester" << CUtil::CLog::endl;
	m_Log << "    HEAD=<head num>        	 - specify head" << CUtil::CLog::endl;
	m_Log << "    LTX_CEX_DEBUG          	 - if set causes debugging info to be printed" << CUtil::CLog::endl;
	m_Log << "    LTX_CEX_VERSION        	 - if set causes version info to be printed" << CUtil::CLog::endl;
	m_Log << "    LTX_CEX_SYNTAX_CHECK   	 - if set causes syntax to be check, commands are NOT executed" << CUtil::CLog::endl;
	m_Log << "    LTX_CEX_TIMEOUT=<secs> 	 - if set causes all readback from the tester " << CUtil::CLog::endl;
	m_Log << "                                 to fail after waiting <secs> seconds." << CUtil::CLog::endl;
	m_Log << "                                 <secs> <= 0 is ignored." << CUtil::CLog::endl;
	m_Log << "" << CUtil::CLog::endl;
	m_Log << "LTX CEX commands:" << CUtil::CLog::endl;
	m_Log << "    (all commands will accept the options -help, -? to get help)" << CUtil::CLog::endl;
	m_Log << "add_stream <client name> <stream name>" << CUtil::CLog::endl;
	m_Log << "cex_help" << CUtil::CLog::endl;
	m_Log << "cex_version" << CUtil::CLog::endl;
	m_Log << "clear_memories <options>" << CUtil::CLog::endl;
	m_Log << "" << CUtil::CLog::endl;
}

/* ------------------------------------------------------------------------------------------
processes all args past -c[ommand]
-	-t[ester] <tester> can still be called after -c[ommand] so it's processed here
-	-c[ommand] arg object will contain the <command> as it's value if a valid 
	<command> is found
-	if -h[elp] is found before any -t[ester] after <command>, we will not connect
	to tester
------------------------------------------------------------------------------------------ */
bool CCmd::scan(std::list< std::string >& Args)
{
	std::list< std::string > a;
	bool bTester = false;
	for (std::list< std::string >::iterator it = Args.begin(); it != Args.end(); it++)
	{ 
		std::string arg( (*it) );

		// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
		// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (arg.compare("-t") == 0) arg = "-tester"; 

		// get a list of (partial) matching args in the valid list of args available on '-command'
		std::vector< CArg* > v;
		findChildren(arg, v, true);

		// is there no match?
		if (!v.size())
		{
			// if there's no partial match and we haven't found <command> yet then this is error
			if (getValue().empty())
			{
				m_Log << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CUtil::CLog::endl;
				return false;			
			}
			// otherwise, this can be possibly one of the parameters for <command>. we don't deal with that.
			// instead, we let <command> deal with it so we just pass it to <command> as one of its parameters
			else
			{
				a.push_back(arg);
				continue;
			}
		}

		// is it ambiguous?
		if (v.size() > 1)
		{
			// in our first attempt to search, we did a partial match. this time, let's do an exact match
			// because this arg might be a <command>
			findChildren(arg, v);

			// is it still ambiguous?
			if (v.size() > 1)
			{			
				m_Log << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
				for (unsigned int i = 0; i < v.size(); i++) m_Log << "'" << v[i]->name() << "', ";
				m_Log << CUtil::CLog::endl;
				return false;
			}
			
			// is there no exact match?
			if (!v.size())
			{
				// if there's no partial match and we haven't found <command> yet then this is error
				if (getValue().empty())
				{
					m_Log << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CUtil::CLog::endl;
					return false;			
				}
				// otherwise, this can be possibly one of the parameters for <command>. we don't deal with that.
				// instead, we let <command> deal with it so we just pass it to <command> as one of its parameters
				else
				{
					a.push_back(arg);
					continue;
				}				
			}
			// at this point, we found a single exact match. let's handle it in the next codes
		}

		// at this point, we found a unique match. it can be -t[ester], -h[elp], or <command>

		if (v[0]->is("-tester"))
		{
			// expect another argument after this as <tester>
			it++;
			if (it == Args.end())
			{
				m_Log << "CEX Error: CEX arguments: -tester option found but tester name missing." << CUtil::CLog::endl;
				return false;
			}
			else
			{
				// we ignore -t[ester] if -h[elp] is already found
				if (!getValue().empty())
				{
					if ( getChild( getValue() )->getChild("-help")->has("ok") ) 
						continue;
				}
				// otherwise, let's get the <tester> and enable connect
				v[0]->setValue( *it );
				v[0]->getChild("connect")->setValue("ok");
				bTester = true;
				continue;
			}
		}
	
		if (v[0]->is("-help"))  
		{
			// if we haven't found <command> yet, ERROR
			if (getValue().empty())
			{
				m_Log << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CUtil::CLog::endl;
				return false;	
			}
			// if <command> already found, let's enable it's <help>
			else
			{
				// if -h[elp] is found before -t[ester], we disable connection
				if (!bTester)
				{
					CTester& T = CTester::instance();
					T.getChild("connect")->setValue("");					
				}
				// enable help for this <command>
				getChild( getValue() )->getChild("-help")->setValue("ok");
				continue;
			}
		}

		// at this point, it might be the <command> but it might have been a partial match only.
		// <command> must be an exact match. let's test it
		findChildren(arg, v);
		
		// since we already know that partial search returns unique match, we only test for non match on exact search
		if (!v.size())
		{
			m_Log << "CEX Error: " << arg << ": '" << arg << "' is not a CEX command. " << CUtil::CLog::endl;
			return false;			
		}
		// match is found and we can guarantee it's a unique (exact) match at this point
		else setValue(arg);	
	}

	// after scanning all args after '-command', let's check if there's a valid <command>
	if (!getValue().empty())
	{
		CArg* pCmd = getChild( getValue() );
		if (pCmd) return pCmd->scan(a);
	}

	// if there's no <command> then we don't do anything. let tester go into loop mode.
	return true;
}

/* ------------------------------------------------------------------------------------------
execute get_head
-	there should be no options for this arg
------------------------------------------------------------------------------------------ */
bool CGetHead::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "**********************************************************************" << CUtil::CLog::endl;
		m_Log << "L T X                         get_head                           L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
	 	m_Log << "       get_head - get_head prints the number of the head currently " << CUtil::CLog::endl;
		m_Log << "		   in use." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
	 	m_Log << "       get_head" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
	 	m_Log << "       The command get_head prints the head number that the tester " << CUtil::CLog::endl;
		m_Log << "	is currently using. " << CUtil::CLog::endl;
		m_Log << "**********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	if (m_Args.size())
	{		
		m_Log << "CEX Error: cex_version: does not accept parameters. Found '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		m_Log << "CEX: Head number " << CTester::instance().getHead() << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute cex_version
-	there should be no options
------------------------------------------------------------------------------------------ */
bool CCexVersion::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "***********************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                         cex_version                        L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        cex_version - print version information" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        cex_version" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        Print the version information. " << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "***********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	if (m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown option '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "**********************************************************************" << CUtil::CLog::endl;
		m_Log << "CEX Apps Version Developed by Allan Asis. Rev 0.1" << CUtil::CLog::endl;
		m_Log << "Build Date: " << CUtil::CLog::endl;
		m_Log << "Build OS: " << CUtil::CLog::endl;
		m_Log << "**********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute get_name
------------------------------------------------------------------------------------------ */
bool CGetName::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "********************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                         get_name                        L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_name - get_name prints the name of the current tester." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_name" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_name prints the name of the tester that is" << CUtil::CLog::endl;
		m_Log << "        currently being used." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	if (m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown parameter '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		m_Log << "CEX: Name of Tester : " << CTester::instance().Tester()->getName() << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute get_username
-	there should be no option
------------------------------------------------------------------------------------------ */
bool CGetUserName::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           get_username                          L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}
	
	if (m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown parameter '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		m_Log << "CEX: Current session owner: " << CTester::instance().ProgCtrl()->getUserName() << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute program_loaded
------------------------------------------------------------------------------------------ */
bool CProgramLoaded::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           program_loaded                          L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}
	if (m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown option '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		CTester& T = CTester::instance();
		if (T.ProgCtrl()->isProgramLoaded()) m_Log << "CEX: Program " << T.ProgCtrl()->getProgramName() << " is currently loaded." << CUtil::CLog::endl;
		else m_Log << "CEX: There is currently no program loaded." << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute program_load_done
------------------------------------------------------------------------------------------ */
bool CProgramLoadDone::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           program_load_done                      L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	if (m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown option '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	else
	{
		CTester& T = CTester::instance();
		if (T.ProgCtrl()->isProgramLoadDone()) m_Log << "CEX: Program " << T.ProgCtrl()->getProgramName() << " is done loading." << CUtil::CLog::endl;
		else m_Log << "CCEX: Program is not done loading." << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute load
------------------------------------------------------------------------------------------ */
bool CLoad::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "*****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                            load                                  L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        load - load the specified program into the tester" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        load <pathname> [-display]" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The load command sends the indicated file to the tester to be" << CUtil::CLog::endl;
		m_Log << "        loaded as a Cadence test program. If the file cannot be  opened" << CUtil::CLog::endl;
 		m_Log << "       or  loaded,  or  has an incorrect \"VTI  Version\" stamp, an error" << CUtil::CLog::endl;
 		m_Log << "       message results. " << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        If \"-display\" is given, Cex will display all the applicable windows " << CUtil::CLog::endl;
		m_Log << "        with the load including errorTool, tpa_server, led, and binTool." << CUtil::CLog::endl;
		m_Log << "        By default, only the errorTool icon will be visible." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "*****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	std::vector< std::string > v;
	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{ 
		std::string arg( (*it) );
		CArg* p = getChild(arg);
		// is param not a valid arg? then it might be the test program path to load
		if (!p)	v.push_back( arg );
		// or it can be a valid arg. valid arg must be exact match
		else
		{ 
			if (p->is("-display")) p->setValue("ok"); 
		}
	}	

	// if multiple programs are specified
	if (v.size() > 1)
	{
		m_Log << "CEX Error: load: Multiple program names found, ";
		for (unsigned int i = 0; i < v.size(); i++){ m_Log << "'" << v[i] << "', "; }
		m_Log << CUtil::CLog::endl;
		return false;
	}
	// if no program is specified
	else if (!v.size())
	{
		m_Log << "CEX Error: load: Missing test program name (ltx/cex)" << CUtil::CLog::endl;
		return false;
	}
	// found only one program as option, we store that
	else setValue( v[0] );

	CTester& T = CTester::instance();

	// is there any program loaded? if yes, ERROR
	if (T.ProgCtrl()->isProgramLoaded())
	{
		m_Log << "CEX Error: Another program '" << T.ProgCtrl()->getProgramName() << "' is already loaded." << CUtil::CLog::endl;
		return false;
	}

	// let's load program!
	m_Log << "CEX: Program " << getValue() << " is loading " << (getChild("-display")->has("ok")? "WITH" : "WITHOUT") << " display..." << CUtil::CLog::endl;
	T.ProgCtrl()->load( getValue().c_str(), EVXA::WAIT, getChild("-display")->has("ok")? EVXA::DISPLAY : EVXA::NO_DISPLAY );

	// did something bad happend when we tried to load program?
	if ( T.ProgCtrl()->getStatus() != EVXA::OK )
	{
		m_Log << "CEX Error: Error in loading " << getValue()  << CUtil::CLog::endl;
		return false;
	} 
	// can we check if program is actually loaded?
	if ( T.ProgCtrl()->isProgramLoaded() )
	{
		m_Log << "CEX: Loaded program " << T.ProgCtrl()->getProgramName() << "." << CUtil::CLog::endl;
		return true;
	}
	else
	{
		m_Log << "CEX Error: There is no program loaded." << CUtil::CLog::endl;
		return false;
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
scan options for unload
-	-nowait = false by default
-	-dontsave = false by default
-	nWait is stored in -wait arg object
-	if -wait value is empty then we have not receive a -wait option
-	-wait option is default and nWait = 0 is default as well
-	if -wait 0, it waits forever
------------------------------------------------------------------------------------------ */
bool CUnload::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "********************************************************************" << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        unload - unload the loaded program" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "               unload [-wait <seconds> | -nowait] [-dontsave]" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        unload  removes  the  currently  loaded  Test Program, if one is" << CUtil::CLog::endl;
		m_Log << "        loaded, running  its on_unload procedure if it has one. " << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        The default syntax is \"unload\" with no arguments, which performs" << CUtil::CLog::endl;
		m_Log << "        the unload, prompts to save the Cadence program if necessary, and" << CUtil::CLog::endl;
		m_Log << "        waits forever until the unload is finished." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        The \"-wait\" flag allows the user to specify the number of seconds" << CUtil::CLog::endl;
		m_Log << "        to wait for the program unload before timing out. If this flag is " << CUtil::CLog::endl;
		m_Log << "        not specified, the program defaults to wait mode with no timeout." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        The \"-nowait\" flag can be used to force the process to return " << CUtil::CLog::endl;
		m_Log << "        without waiting for the unload command to complete." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        The \"-dontsave\" flag disables the prompt for saving the Cadence " << CUtil::CLog::endl;
		m_Log << "        program if it has changed since the last save." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	// set default values
	getChild("-nowait")->setValue(""); // false. waiting by default
	getChild("-dontsave")->setValue(""); // false. saving by default
	
	// let's find any invalid arg
	std::vector< std::string > v;
	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{ 
		std::string arg( (*it) );
		CArg* p = getChild(arg);

		// is this option not valid? error then...
		if (!p) v.push_back(arg);

		// or it can be a valid arg. valid arg must be exact match
		else
		{
			// if -wait is found, let's take the next param as the <wait> value.
			if (p->is("-wait"))
			{
				// if '-nowait' is also an option used then it's an error
				if (getChild("-nowait")->has("ok"))
				{
					m_Log << "CEX Error: unload: No-wait with wait interval not available." << CUtil::CLog::endl;
					return false;
				}
				// is there no more argument after '-wait'?
				it++;
				if (it == m_Args.end())
				{
					m_Log << "CEX Error: unload: 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}
				// is the argument after '-wait' a number?
				if ( !CUtil::isInteger( (*it) ) )
				{
					m_Log << "CEX Error: unload: '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}		
				// let's get the number and store it in -wait arg object
				//nWait = toLong( (*it) );
				p->setValue( (*it) );		
				
				// we also set -nowait arg object as disabled
				getChild("-nowait")->setValue("");
				continue;								
			}
			
			// found '-nowait' param
			if (p->is("-nowait"))
			{
				// if '-wait' is also an option used then it's an error
				if (!getChild("-wait")->getValue().empty())
				{
					m_Log << "CEX Error: unload: No-wait with wait interval not available." << CUtil::CLog::endl;
					return false;
				}
				// remove the value set in -wait (to be sure it's cleared
				getChild("-wait")->setValue("");
				// enable -nowait arg object
				p->setValue( "ok" );	
				continue;						
			}
			// found -dontsave param
			if (p->is("-dontsave"))
			{
				p->setValue("ok");
				continue;
			}
		}		
	}

	// did we find invalid args?
	if (v.size())
	{
		m_Log << "CEX Error: unload: Unknown parameter '" << v[0] << "'." << CUtil::CLog::endl;
		return false;
	}

	// EXECUTION time!
	CTester& T = CTester::instance();
	bool bWait = !getChild("-nowait")->has("ok");		
	long nWait = getChild("-wait")->getValue().empty()? 0 : CUtil::toLong( getChild("-wait")->getValue() );
	bool bDontSave = getChild("-dontsave")->has("ok");

	// unload the program
	if ( !T.ProgCtrl()->isProgramLoaded() )
	{
		m_Log << "CEX Error: There is no program loaded." << CUtil::CLog::endl;
		return false;
	}

	const char* szProgramName =  T.ProgCtrl()->getProgramName();
	m_Log << "CEX: Program " << szProgramName << " is unloading. This may take a few moments...." << CUtil::CLog::endl;

	// execute evxa command to unload program
	// if nWait = 0, it will wait forever
	// bSave is ignored by evxa and original CEX command so must figure a work around to implement this behavior
	T.ProgCtrl()->unload( bWait? EVXA::WAIT : EVXA::NO_WAIT, nWait, bDontSave );

	if ( T.ProgCtrl()->getStatus() != EVXA::OK )
	{
		m_Log << "CEX Error: Error in unloading " << szProgramName << CUtil::CLog::endl;
		return false;
	}

	if ( !T.ProgCtrl()->isProgramLoaded() || !bWait)
	{
		m_Log << "CEX: Unloaded program " << szProgramName << "." << CUtil::CLog::endl;
		return true;
	}
	else
	{
		m_Log << "CEX Error: Program " << T.ProgCtrl()->getProgramName() << " is still loaded." << CUtil::CLog::endl;
		return false;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
handle start command
- 	default is -wait <0>
- 	default is execute start only once and will not log loop count message
-	if -nowait -wait <t>, -nowait is ignored.
-	if -wait <t> -nowait, it's ERROR
-	if -nowait -wait <t> -nowait, it's ERROR
-	if -ntimes <n> is used, -nowait is ignored no matter when it's used
- 	if -wait <t> is used multiple times, the last one gets used.
- 	if -ntimes <n> is used multiple times, the last one gets used.
-	n in ntimes <n> has min value of 1. any value lower than this is set to 1.
------------------------------------------------------------------------------------------ */
bool CStart::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "***********************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                         start                              L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        start - run the test program beginning with the on_start procedure" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        start [-ntimes <loop count> [-wait<seconds>]]" << CUtil::CLog::endl;
		m_Log << "              [-nowait]" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        start  begins execution of the loaded program with the procedure" << CUtil::CLog::endl;
		m_Log << "        on_start. The  optional -ntimes <loop count> specifies how  many" << CUtil::CLog::endl;
 		m_Log << "       times  the  program  is  to be  executed. If -wait is specified," << CUtil::CLog::endl;
		m_Log << "        cex waits the specified number of seconds   before  issuing  the" << CUtil::CLog::endl;
		m_Log << "        next  start.  Here  the  looping  is  done  by  cex,  not by the" << CUtil::CLog::endl;
		m_Log << "        tester. cex waits for the  program  to  be  started  loop  count" << CUtil::CLog::endl;
		m_Log << "        times  before  terminating.   If  -nowait is specified, cex puts" << CUtil::CLog::endl;
 		m_Log << "       the  request  to  run  into  the  tester's  command   queue  and" << CUtil::CLog::endl;
 		m_Log << "       terminates,  freeing  the  shell  for  use.  Note  that  -nowait" << CUtil::CLog::endl;
 		m_Log << "       cannot be used with -ntimes. " << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        Example 1: Start the test program run without waiting for a response" << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "             cex -t <tester> -c start -nowait" << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        Example 2: Run the test program for 10 loops, waiting 2 seconds " << CUtil::CLog::endl;
		m_Log << "                   between runs" << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "             cex -t <tester> -c start -ntimes 10 -wait 2" << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
 		m_Log << "       Example 3:" << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "        If you would like to use -ntimes but still be able to work in " << CUtil::CLog::endl;
		m_Log << "        the same shell, run the cex command in the background with the  " << CUtil::CLog::endl;
		m_Log << "        ampersand symbol \"&\". For example:" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "             cex -t <tester> -c start -ntimes 2000 -wait 4 &" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The above command runs the currently loaded program 2000  times," << CUtil::CLog::endl;
		m_Log << "        waiting  4  seconds   between  runs. This occurs as a background" << CUtil::CLog::endl;
		m_Log << "        process and the  shell  prompt  returns   immediately.  See  the" << CUtil::CLog::endl;
		m_Log << "        manual  for  your specific shell regarding control of background" << CUtil::CLog::endl;
		m_Log << "        processes. " << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "********************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	// defaults
	m_bLoop = false; // no loop
	m_nLoop = 1; // execute once	
	m_nWaitAfterExec = 0; // wait time in <sec> after each execution
	m_bExitAfterExec = false; // -nowait flag; 
	m_bWaitAfterExec = false; // -wait flag

	// let's find any invalid arg
	std::vector< std::string > v;
	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{ 
		std::string arg( (*it) );
		CArg* p = getChild(arg);

		// is this option not valid? error then...
		if (!p) v.push_back(arg);

		// or it can be a valid arg. valid arg must be exact match
		else
		{
			// if -wait is found, let's take the next param as the <wait> value.
			if (p->is("-wait"))
			{
				// is there no more argument after '-wait'?
				it++;
				if (it == m_Args.end())
				{
					m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}
				// is the argument after '-wait' a number?
				if ( !CUtil::isInteger( (*it) ) )
				{
					m_Log << "CEX Error: " << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}		
		
				// let's now disable  -nowait flag if it was set prior to this
				m_bExitAfterExec = false;

				// let's also get the number as wait in <sec>, ensure it's min as 0
				m_nWaitAfterExec = CUtil::toLong( (*it) );	
				m_nWaitAfterExec = m_nWaitAfterExec < 0 ? 0 : m_nWaitAfterExec;

				// enable wait after execution
				m_bWaitAfterExec = true;	
				continue;								
			}
			
			// found '-nowait' param
			if (p->is("-nowait"))
			{
				// if -wait <sec> is already found prior to this,it's error
				if ( m_bWaitAfterExec )
				{
					m_Log << "CEX Error: " << name() << ": No-wait with wait interval not available." << CUtil::CLog::endl;
					return false;
				}
				// if -ntimes is already called prior to this, we ignore -nowait
				if (m_bLoop) continue;

				// otherwise, let's do a -nowait execution
				m_bExitAfterExec = true;							
				continue;
			}

			// found -ntimes param
			if (p->is("-ntimes"))
			{
				// is there no more argument after '-ntimes'?
				it++;
				if (it == m_Args.end())
				{
					m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}
				// is the argument after '-wait' a number?
				if ( !CUtil::isInteger( (*it) ) )
				{
					m_Log << "CEX Error: " << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
					return false;
				}		

				// let's now disable -nowait flag if it was set prior to this
				m_bExitAfterExec = false;

				// let's set loop to true 
				m_bLoop = true;

				// let's also get the number as number of loops, ensure it's min as 1
				m_nLoop = CUtil::toLong( (*it) );	
				m_nLoop = m_nLoop < 1 ? 1 : m_nLoop;				
				continue;
			}
		}		
	}

	// did we find invalid args?
	if (v.size())
	{
		m_Log << "CEX Error: " << name() << ": Unknown parameter '" << v[0] << "'." << CUtil::CLog::endl;
		return false;
	}	

	// EXECUTION time!
	CTester& T = CTester::instance();
	for (int i = 0; i < m_nLoop; i++)
	{
		if (m_bLoop) m_Log << "Looping (" << i + 1 << "/" << m_nLoop << ")" << CUtil::CLog::endl; // replicate original CEX printing this on loop

		T.ProgCtrl()->start( m_bExitAfterExec? EVXA::NO_WAIT : EVXA::WAIT );

		// if -nowait, we immediately exit after executing
		if (m_bExitAfterExec) return true;
	
		// -wait <sec> occurs AFTER every execution
		if (m_nWaitAfterExec) sleep(m_nWaitAfterExec);
	}
	
	return true;
}


/* ------------------------------------------------------------------------------------------
execute get_exp
-	example expression: TestProgData.Device
------------------------------------------------------------------------------------------ */
bool CGetExp::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           get_exp                      L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	if (!m_Args.size())
	{		
		m_Log << "CEX Error: " << name() << ": Unknown option '" << (*m_Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}

	// if there's no param after this command then it's error. we're expecting <expression> to immediately follow -c <get_exp>
	if (!m_Args.size())
	{
		m_Log << "CEX Error: " << name() << ": Missing expression name." << CUtil::CLog::endl;
		return false;
	}

	// if there's no param  after <expression> then it's error. we're expecting one of the -display options
	if (m_Args.size() < 2)
	{
		m_Log << "CEX Error: " << name() << ": Missing mode name." << CUtil::CLog::endl;
		return false;
	}

	// we strictly expect only 2 arguments after this command - <expression> and -display. anything else is error.
	if (m_Args.size() > 2)
	{
		m_Log << "CEX Error: " << name() << ": Multiple mode names found - ";
		for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++) m_Log << "'" << (*it) << "', ";
		m_Log << CUtil::CLog::endl;
		return false;
	}

	// store the expression to this object
	std::list< std::string >::iterator it = m_Args.begin();
	setValue( (*it) );

	// check if mode name is a valid mode.
	CArg* pMode = getChild( *(++it) );
	if (!pMode)
	{
		m_Log << "CEX Error: Unknown display mode type. " << (*it) <<  CUtil::CLog::endl;
		return false;
	}
	else pMode->setValue("ok");

	// EXECUTION time!
	CTester& T = CTester::instance();
	EVX_EXPR_DISPLAY_MODE nDisplayMode;
	if ( getChild("expression")->has("ok") ) nDisplayMode = EVX_SHOW_EXPRESSION;
	if ( getChild("value")->has("ok") ) nDisplayMode = EVX_SHOW_VALUE;
	if ( getChild("multi_value")->has("ok") ) nDisplayMode = EVX_SHOW_MULTI_VALUE;
	if ( getChild("multi_range")->has("ok") ) nDisplayMode = EVX_SHOW_MULTI_RANGE;
	m_Log << T.ProgCtrl()->getExpression( getValue().c_str(), nDisplayMode) << CUtil::CLog::endl;
	
	return true;
}

/* ------------------------------------------------------------------------------------------
scan args and execute evx_summary
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
bool CEvxSummary::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           evx_summary                      L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	// if there's no options, then it's just a query
	if (!m_Args.size()) return true;
  
	// the first option must be valid and will be considered as primary option. 
	std::list< std::string >::iterator it = m_Args.begin();
	CArg* pSummaryType = getChild( *it );
	if (!pSummaryType)
	{
		m_Log << "CEX Error: evx_summary: " << (*it) << " is not a valid option." << CUtil::CLog::endl;
		return false;
	}
	// a valid option for evx_summary is found, let's enable this option and move to next arg
	else
	{
		pSummaryType->setValue("ok");
		setValue( *it );
		it++;
	}

	// now let's deal with the succeeding args. these args are expected to be -c evx_summary <opt> and their corresponding options if any
	CArg* pCurr = pSummaryType;
	while ( it != m_Args.end() )
	{
		CArg* pNext = pCurr->getChild( *it );
	
		// if this arg is invalid for this <opt>...
		if (!pNext)
		{
			// if the <opt> we're currently searching for is already -c evx_summary <opt>, there's nothing to search anymore
			if (pCurr == pSummaryType) break;
			// otherwise, let's move up one <opt> layer
			else pCurr = pCurr->parent();
		}
		// if this arg is a valid <opt>, let's enable it and move to next arg. also move to next <opt> layer
		else
		{
			// handle special case: -c evx_summary output <opt> can have multiple <opt>. the last valid one gets dibs.
			if (pCurr->is("output"))
			{
				if (pNext->is("lot")) pCurr->getChild("sublot")->setValue("");
				if (pNext->is("sublot")) pCurr->getChild("lot")->setValue("");
			}

			pNext->setValue("ok");
			it++;
			pCurr = pNext;
		}
	}
	//printRecursive(this);

	// if there's still arg not processed at this point, then that's considered invalid. for some <opt> invalid arg is ERROR
	// we want to log error due to this invalid arg during execution so we save it first
	if (it != m_Args.end()) m_strInvalidArg = *it;

	// EXECUTION time!
	CTester& T = CTester::instance();
	if (has("site"))
	{
		// do the job. note that we only care about first option of <site>
		if ( getChild("site")->getChild("on")->has("ok") ) T.ProgCtrl()->setSummary(EVX_UpdateBreakout, EVXA::ON);		
		else if ( getChild("site")->getChild("off")->has("ok") ) T.ProgCtrl()->setSummary(EVX_UpdateBreakout, EVXA::OFF);
		else T.ProgCtrl()->setSummary(EVX_UpdateBreakout, T.ProgCtrl()->getSummary(EVX_UpdateBreakout) == EVXA::ON? EVXA::OFF : EVXA::ON);

		// display results 
		m_Log << "CEX: evx_summary " << getValue() << " option has been ";
		m_Log << ( (getChild("site")->getChild("on")->has("ok") || getChild("site")->getChild("off")->has("ok") )? "set":"toggled") << " to ";
		m_Log << (T.ProgCtrl()->getSummary(EVX_UpdateBreakout) == EVXA::ON? "ON" : "OFF") << "." << CUtil::CLog::endl;
		return true;
	}

	else if (has("clearfinal") || has("clearpartial"))
	{
		if (has("clearfinal")) T.ProgCtrl()->clearFinalSummary();
		else T.ProgCtrl()->clearPartialSummary();
		m_Log << "CEX: cleared "<< (has("clearfinal")? "final":"partial") << " summary." << CUtil::CLog::endl;
		return true;
	}	
	
	else if (has("partial") || has("final"))
	{
		// get the summary option <opt> object
		CArg* pSummary = getChild( getValue() );
		for(unsigned int i = 0; i < pSummary->getNumChildren(); i++)
		{ 
			CArg* pOpt = pSummary->getChild(i);
			if (!pOpt->has("ok")) continue;			

			// let's specify which summary type are we going to process			
			EVX_SUMMARY_TYPE st;
			if ( pSummary->is("partial") && pOpt->is("full") ) st = EVX_UpdateFinal; // UpdateFinal - partial full
			else if ( pSummary->is("partial") && pOpt->is("clear") ) st = EVX_ClearPartial; // ClearPartial - partial clear
			else if ( pSummary->is("final") && pOpt->is("clear") ) st = EVX_ClearFinal; // ClearFinal - final clear
			else if ( pOpt->is("site") ) st = EVX_UpdateBreakout; // UpdateBreakout - site
			else continue;

			// do the job
			if ( pOpt->getChild("on")->has("ok") ) T.ProgCtrl()->setSummary(st, EVXA::ON);
			else if ( pOpt->getChild("off")->has("ok") ) T.ProgCtrl()->setSummary(st, EVXA::OFF);
			else T.ProgCtrl()->setSummary(st, T.ProgCtrl()->getSummary(st) == EVXA::ON? EVXA::OFF : EVXA::ON);

			// display results 
			m_Log << "CEX: evx_summary ";
			m_Log << (pOpt->is("site")? "": pSummary->name());
			m_Log << (pOpt->is("site")? "": " ") << pOpt->name();
			m_Log << " option has been " << ( (pOpt->getChild("on")->has("ok") || pOpt->getChild("off")->has("ok"))? "set":"toggled") << " to ";
			m_Log << (T.ProgCtrl()->getSummary(st) == EVXA::ON? "ON" : "OFF") << "." << CUtil::CLog::endl;
		}
		// do we have invalid arg?
		if ( m_strInvalidArg.length() )
		{
			m_Log << "CEX Error: " << name() << ": " << m_strInvalidArg << " is not a valid " << getValue() << " summary type." << CUtil::CLog::endl;
			return false;
		}
		return true;
	}

	else if (has("output"))
	{
		// do we have invalid arg?
		if ( m_strInvalidArg.length() )
		{
			m_Log << "CEX Error: " << name() << ": " << m_strInvalidArg << " is not a valid " << getValue() << " summary type." << CUtil::CLog::endl;
			return false;
		}

		// get settings from arg options
		if (getChild("output")->getChild("lot")->has("ok")) T.ProgCtrl()->setLotTypeSummary(EVX_LOT_SUMMARY);
		if (getChild("output")->getChild("sublot")->has("ok")) T.ProgCtrl()->setLotTypeSummary(EVX_SUBLOT_SUMMARY);
		if (getChild("output")->getChild("final")->has("ok")){ T.ProgCtrl()->setSummary(EVX_ClearFinal, EVXA::ON); T.ProgCtrl()->setSummary(EVX_ClearPartial, EVXA::OFF); }
		if (getChild("output")->getChild("partial")->has("ok")){ T.ProgCtrl()->setSummary(EVX_ClearFinal, EVXA::OFF); T.ProgCtrl()->setSummary(EVX_ClearPartial, EVXA::ON); }

		// do the job
		bool bFinal = (T.ProgCtrl()->getSummary(EVX_ClearFinal) == EVXA::ON)? true:false;
		bFinal? T.ProgCtrl()->outputFinalSummary() : T.ProgCtrl()->outputPartialSummary();

		// display results
		m_Log << CUtil::CLog::endl << "CEX: " << name() << " output -- " << (bFinal? "Final" : "Partial") << "/" << (T.ProgCtrl()->getLotTypeSummary() == EVX_LOT_SUMMARY? "Lot":"Sublot") << CUtil::CLog::endl;
		if (bFinal)
		{
			m_Log << "     Clearing results: Sublot" << (T.ProgCtrl()->getLotTypeSummary() == EVX_LOT_SUMMARY? ", Lot":"") << CUtil::CLog::endl;
			m_Log << "     Reseting NextSerial to '1'." << CUtil::CLog::endl;
		}
		m_Log << CUtil::CLog::endl;
		return true;
	}
	// if there's no options, then it's just a query
	else
	{
		m_Log << "evx_summary status:" << CUtil::CLog::endl;

		// print site state
		EVXA::ON_OFF_TYPE state = T.ProgCtrl()->getSummary(EVX_UpdateBreakout);
		m_Log << "    site     " << (state == EVXA::ON? "on" : "off") << CUtil::CLog::endl;
		// print lot type
		EVX_LOT_TYPE_SUMMARY lot = T.ProgCtrl()->getLotTypeSummary();
		m_Log << "    lot_type " << (lot == EVX_SUBLOT_SUMMARY? "sublot" : "lot") << CUtil::CLog::endl;

		// print partial status
		m_Log << "    partial  full " << (T.ProgCtrl()->getSummary(EVX_UpdateFinal) == EVXA::ON? "on" : "off");
		m_Log << ",  clear " << (T.ProgCtrl()->getSummary(EVX_ClearPartial) == EVXA::ON? "on" : "off") << CUtil::CLog::endl;

		// print final status
		m_Log << "    final    clear " << (T.ProgCtrl()->getSummary(EVX_ClearFinal) == EVXA::ON? "on" : "off") << CUtil::CLog::endl;
	}
	return true;
}

void printRecursive( CArg* pCurr )
{
	for (unsigned int i = 0; i < pCurr->getNumChildren(); i++)
	{
		CArg* pNext = pCurr->getChild(i);
		if (pNext->has("ok")) std::cout << (pNext->parent()? pNext->parent()->name(): "") << (pNext->parent()? ":":"") << pNext->name() << std::endl;	
		printRecursive( pNext );
	}
}

/* ------------------------------------------------------------------------------------------
evx_dlog_methods [ <dlog_index> ]
-	strictly accepts only 1 arg.
-	the arg must be integer and within dlog index range
-	if no arg is given, will process all available dlog 
------------------------------------------------------------------------------------------ */
bool CDlogMethods::exec()
{
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                           evx_dlog_methods                          L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	// strictly accept only 1 argument
	if (m_Args.size() > 1)
	{		
		m_Log << "CEX Error: "<< name() << ": Too many arguments." << CUtil::CLog::endl;
		return false;
	}

	// at this point, we know there's 1 arg. store it
	if (m_Args.size()) setValue( *m_Args.begin() );

	// EXECUTION time!
	CTester& T = CTester::instance();
	// do we arg? if yes, it must be integer and has valid range
	if ( getValue().size() )
	{
		if ( !CUtil::isInteger( getValue() ) )
		{
			m_Log << "CEX Error: expect an positive integer number for dlog index." << CUtil::CLog::endl;
			return false;
		}
		
		// check if <dlog_index> is within range
		long n = CUtil::toLong( getValue() );
		if ( n < 0 || n >= T.ProgCtrl()->getNumDatalogs() )
		{
			m_Log << "CEX Error: valid dlog index is from 0 to " << (T.ProgCtrl()->getNumDatalogs() - 1) << "." << CUtil::CLog::endl;
			return false;
		}
	}

	for (int i = 0; i < T.ProgCtrl()->getNumDatalogs(); i++)
	{
		// try to get the <method> at this index
		std::stringstream val;
		// if <method> is immediate, you can query it at this index
		val << T.ProgCtrl()->getDatalogString(i, 1, 0);

		// but if <method> is buffered, it must be queried here
		if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);

		// if we didn't find <method>...
		if (!val.str().size())
		{
			if ( getValue().size() )
			{
				if ( CUtil::toLong( getValue() ) == i ) 
				{
					m_Log << "No evx datalog method associated with dlog " << i << CUtil::CLog::endl;
					break;			
				}
			}
			else m_Log << "No evx datalog method associated with dlog " << i << CUtil::CLog::endl;
		}
		// if we found <method>, let's print its details
		else
		{
			if ( getValue().size() )
			{
				if (  CUtil::toLong( getValue() ) == i ) 
				{
					m_Log << "Evx datalog "<< i << " method: "<< val.str() << CUtil::CLog::endl;
					break;		
				}	
			}
			else m_Log << "Evx datalog "<< i << " method: "<< val.str() << CUtil::CLog::endl;
		}
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
evx_dlog_file_freq [-m <method> | -n <dlog_index> ] <file freq> 
-	must have a valid arg
-	-n <n> can be used multiple times, but last one gets the dibs
-	-m <method> can be used multiple times, but last one gets the dibs
- 	must used either -n or -m BUT can only use one of them
-	if -m <method> is used, and there's multiple dlogs that uses the same <method>,
	the first dlog gets updated. the rest are unchanged.
-	only 1 dlog freq is accepted. more than 1 will be ERROR
------------------------------------------------------------------------------------------ */
bool CDlogFileFreq::exec()
{
	// if -help, let's do it and exit
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                          evx_dlog_file_freq                         L T X" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " NAME" << CUtil::CLog::endl;
		m_Log << "        get_username - get_username prints the current session owner." << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		m_Log << " SYNOPSIS" << CUtil::CLog::endl;
		m_Log << "        get_username" << CUtil::CLog::endl;
		m_Log << "        " << CUtil::CLog::endl;
		m_Log << "        The command get_username prints the current session owner identified" << CUtil::CLog::endl;
		m_Log << "        by the login name." << CUtil::CLog::endl;
		m_Log << "" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	CTester& T = CTester::instance();

	// if no options, ERROR: CEX Error: Must specify either a valid method or dlog index.
	if (!m_Args.size())
	{		
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		CArg* p = getChild( *it );

		// this arg does not match any of the valid options for this command
		if (!p)
		{
			m_Log << "CEX Error: Frequency must be Lot, SubLot, Wafer, or Session." << CUtil::CLog::endl;
			return false;
		}

		if (p->is("-n"))
		{
			// if no arg after -n, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg is not an integer, ERROR
			if (!CUtil::isInteger( *it ))
			{
				m_Log << "CEX Error: "  << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg's value is within dlog method's range
			if ( CUtil::toLong( *it ) < 0 || CUtil::toLong( *it ) >= T.ProgCtrl()->getNumDatalogs() )
			{
				m_Log << "CEX Error: valid dlog index is from 0 to " << (T.ProgCtrl()->getNumDatalogs() - 1) << "." << CUtil::CLog::endl;
				return false;
			}	
			// at this point, -n <m> is valid, we save it. 
			p->setValue( *it );
		}
		else if (p->is("-m"))
		{
			// if no arg after -m, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: Must specify a valid dlog method with '" << name() << "'." << CUtil::CLog::endl;
				return false;
			}
			// at this point, -m <method> is valid, we save it. 
			p->setValue( *it );
		}
		// it might be any of the valid file frequency values - Lot, SubLot, Wafer, Session
		else
		{
			// if there's more arg after this, ERROR
			if (++it != m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": Too many arguments. "<< CUtil::CLog::endl;
				return false;
			}
			p->setValue( "ok" ); 
			setValue( *(--it) ); // store here for ease of access
		}						
	}

	// must used either -n or -m BUT can only use one of them
	if ( getChild("-n")->getValue().empty() == getChild("-m")->getValue().empty() )
	{
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	// did we have a valid file frequency value?
	if ( !getChild("Lot")->has("ok") && !getChild("SubLot")->has("ok") && !getChild("Wafer")->has("ok") && !getChild("Session")->has("ok") )
	{
		m_Log << "CEX Error: Frequency must be Lot, SubLot, Wafer, or Session." << CUtil::CLog::endl;
		return false;
	}

	// if -n <n> is used
	if ( getChild("-n")->getValue().size() )
	{
		int i = CUtil::toLong( getChild("-n")->getValue() );
		std::string s = getValue();
		s.insert(0, "DlogFreq:");

		// check if this is valid dlog method
		std::stringstream val;
		val << T.ProgCtrl()->getDatalogString(i, 1, 0);
		if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);

		if (!val.str().size())
		{
			m_Log << "CEX Error: Dlog index " << i << " is not associated with any methods." << CUtil::CLog::endl;
			return false;
		}
		else 
		{
			T.ProgCtrl()->setDatalogFileFreq (i, s.c_str() );
			m_Log << "CEX: File frequency for dlog" << i << " set to " << s << "." << CUtil::CLog::endl;
			return true;
		}
	}
	// if -m <method> is used
	else 
	{
		for (int i = 0; i < T.ProgCtrl()->getNumDatalogs(); i++)
		{
			std::stringstream val;
			val << T.ProgCtrl()->getDatalogString(i, 1, 0);
			if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
			
			if (val.str().compare( getChild("-m")->getValue() ) == 0)
			{
				std::string s = getValue();
				s.insert(0, "DlogFreq:");
				T.ProgCtrl()->setDatalogFileFreq (i, s.c_str());
				m_Log << "CEX: File Frequency for method "<<  getChild("-m")->getValue() << " [dlog" << i << "] set to " << s << "." << CUtil::CLog::endl;
				return true;
			}
		}
		m_Log << "CEX Error: The datalogging method " << getChild("-m")->getValue() << " was not found." << CUtil::CLog::endl;
		return false;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
evx_dlog_sample_rate [-m <method> | -n <dlog_index>] <n>
-	must have a valid arg
-	-n <n> can be used multiple times, but last one gets the dibs
-	-m <method> can be used multiple times, but last one gets the dibs
- 	must used either -n or -m BUT can only use one of them
-	if -m <method> is used, and there's multiple dlogs that uses the same <method>,
	the first dlog gets updated. the rest are unchanged.
-	only 1 dlog sample rate is accepted. more than 1 will be ERROR
------------------------------------------------------------------------------------------ */
bool CDlogSampleRate::exec()
{
	// if -help, let's do it and exit
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                      evx_dlog_sample_rate                    L T X" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	CTester& T = CTester::instance();

	// if no options, ERROR: CEX Error: Must specify either a valid method or dlog index.
	if (!m_Args.size())
	{		
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		CArg* p = getChild( *it );

		// this arg does not match any of the valid options, then it must be the sample rate value <n>
		if (!p)
		{
			// if there's more arg after this, ERROR
			if (++it != m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": Too many arguments. "<< CUtil::CLog::endl;
				return false;
			}
			else --it;
			// check if arg is not an integer, ERROR
			if (!CUtil::isInteger( *it ))
			{
				m_Log << "CEX Error: Rate must be a valid positive integer value." << CUtil::CLog::endl;
				return false;
			}
			// check if next arg's value is > 0
			if ( CUtil::toLong( *it ) <= 0 )
			{
				m_Log << "CEX Error: Must include the rate (a positive integer value)." << CUtil::CLog::endl;
				return false;
			}

			// we now have a valid sample rate. store it.
			setValue( *it ); 
			continue;
		}
		// if p is valid, it can only be either -n or -m
		if (p->is("-n"))
		{
			// if no arg after -n, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg is not an integer, ERROR
			if (!CUtil::isInteger( *it ))
			{
				m_Log << "CEX Error: "  << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg's value is within dlog method's range
			if ( CUtil::toLong( *it ) < 0 || CUtil::toLong( *it ) >= T.ProgCtrl()->getNumDatalogs() )
			{
				m_Log << "CEX Error: valid dlog index is from 0 to " << (T.ProgCtrl()->getNumDatalogs() - 1) << "." << CUtil::CLog::endl;
				return false;
			}	
			// at this point, -n <m> is valid, we save it. 
			p->setValue( *it );
		}
		else if (p->is("-m"))
		{
			// if no arg after -m, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: Must specify a valid dlog method with '" << name() << "'." << CUtil::CLog::endl;
				return false;
			}
			// at this point, -m <method> is valid, we save it. 
			p->setValue( *it );
		}						
	}

	// must used either -n or -m BUT can only use one of them
	if ( getChild("-n")->getValue().empty() == getChild("-m")->getValue().empty() )
	{
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	// is there an <n> arg?
	if ( getValue().empty() )
	{
		m_Log << "CEX Error: Must include the rate (a positive integer value)." << CUtil::CLog::endl;
		return false;
	}

	// if -n <n> is used
	if ( getChild("-n")->getValue().size() )
	{
		int i = CUtil::toLong( getChild("-n")->getValue() );

		// check if this is valid dlog method
		std::stringstream val;
		val << T.ProgCtrl()->getDatalogString(i, 1, 0);
		if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
 
		if (!val.str().size())
		{
			m_Log << "CEX Error: Dlog index " << i << " is not associated with any methods." << CUtil::CLog::endl;
			return false;
		}
		else 
		{
			T.ProgCtrl()->setSampleFreqForStream (i, getValue().c_str() );
			const char* p = T.ProgCtrl()->getSampleFreqForStream(i);
			m_Log << "CEX: Sample rate for dlog" << i << " set to " << (p? p:"") << "." << CUtil::CLog::endl;
			return true;
		}
	}
	// if -m <method> is used
	else 
	{
		for (int i = 0; i < T.ProgCtrl()->getNumDatalogs(); i++)
		{
			std::stringstream val;
			val << T.ProgCtrl()->getDatalogString(i, 1, 0);
			if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
			
			if (val.str().compare( getChild("-m")->getValue() ) == 0)
			{
				T.ProgCtrl()->setSampleFreqForStream (i, getValue().c_str());
				const char* p = T.ProgCtrl()->getSampleFreqForStream(i);
				m_Log << "CEX: Sample rate for method "<<  getChild("-m")->getValue() << " [dlog" << i << "] set to " << (p? p:"") << "." << CUtil::CLog::endl;
				return true;
			}
		}
		m_Log << "CEX Error: The datalogging method " << getChild("-m")->getValue() << " was not found." << CUtil::CLog::endl;
		return false;
	}
	return true;
}


/* ------------------------------------------------------------------------------------------
evx_dlog_testID [-m <method> | -n <dlog_index>] <string>
-	must have a valid arg
-	-n <n> can be used multiple times, but last one gets the dibs
-	-m <method> can be used multiple times, but last one gets the dibs
- 	must used either -n or -m BUT can only use one of them
-	if -m <method> is used, and there's multiple dlogs that uses the same <method>,
	the first dlog gets updated. the rest are unchanged.
-	only 1 dlog testID is accepted. more than 1 will be ERROR
------------------------------------------------------------------------------------------ */
bool CDlogTestID::exec()
{
	// if -help, let's do it and exit
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                          evx_dlog_testID                         L T X" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	CTester& T = CTester::instance();

	// if no options, ERROR: CEX Error: Must specify either a valid method or dlog index.
	if (!m_Args.size())
	{		
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		CArg* p = getChild( *it );

		// this arg does not match any of the valid options for this command
		if (!p)
		{
			// if there's more arg after this, ERROR
			if (++it != m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": Too many arguments. "<< CUtil::CLog::endl;
				return false;
			}
			else --it;

			// this value is the testID. store it.
			setValue( *it ); 
			continue;
		}

		if (p->is("-n"))
		{
			// if no arg after -n, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg is not an integer, ERROR
			if (!CUtil::isInteger( *it ))
			{
				m_Log << "CEX Error: "  << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg's value is within dlog method's range
			if ( CUtil::toLong( *it ) < 0 || CUtil::toLong( *it ) >= T.ProgCtrl()->getNumDatalogs() )
			{
				m_Log << "CEX Error: valid dlog index is from 0 to " << (T.ProgCtrl()->getNumDatalogs() - 1) << "." << CUtil::CLog::endl;
				return false;
			}	
			// at this point, -n <m> is valid, we save it. 
			p->setValue( *it );
		}
		else if (p->is("-m"))
		{
			// if no arg after -m, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: Must specify a valid dlog method with '" << name() << "'." << CUtil::CLog::endl;
				return false;
			}
			// at this point, -m <method> is valid, we save it. 
			p->setValue( *it );
		}					
	}

	// must used either -n or -m BUT can only use one of them
	if ( getChild("-n")->getValue().empty() == getChild("-m")->getValue().empty() )
	{
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	// did we have a testID?
	if ( getValue().empty() )
	{
		m_Log << "CEX Error: Type must be Production or ILQA." << CUtil::CLog::endl;
		return false;
	}

	// if -n <n> is used
	if ( getChild("-n")->getValue().size() )
	{
		int i = CUtil::toLong( getChild("-n")->getValue() );

		// check if this is valid dlog method
		std::stringstream val;
		val << T.ProgCtrl()->getDatalogString(i, 1, 0);
		if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
 
		if (!val.str().size())
		{
			m_Log << "CEX Error: Dlog index " << i << " is not associated with any methods." << CUtil::CLog::endl;
			return false;
		}
		else 
		{
			T.ProgCtrl()->setDatalogType (i, getValue().c_str() );
			const char* p = T.ProgCtrl()->getDatalogType(i);
			m_Log << "CEX: Type for dlog" << i << " set to " << (p?p:"") << "." << CUtil::CLog::endl;
			return true;
		}
	}
	// if -m <method> is used
	else 
	{
		for (int i = 0; i < T.ProgCtrl()->getNumDatalogs(); i++)
		{
			std::stringstream val;
			val << T.ProgCtrl()->getDatalogString(i, 1, 0);
			if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
			
			if (val.str().compare( getChild("-m")->getValue() ) == 0)
			{
				T.ProgCtrl()->setDatalogType (i, getValue().c_str());
				const char* p = T.ProgCtrl()->getDatalogType(i);
				m_Log << "CEX: Type for method "<<  getChild("-m")->getValue() << " [dlog" << i << "] set to " << (p?p:"") << "." << CUtil::CLog::endl;
				return true;
			}
		}
		m_Log << "CEX Error: The datalogging method " << getChild("-m")->getValue() << " was not found." << CUtil::CLog::endl;
		return false;
	}
	return true;
}


/* ------------------------------------------------------------------------------------------
evx_dlog_type [-m <method> | -n <dlog_index> ] <type>
-	must have a valid arg
-	-n <n> can be used multiple times, but last one gets the dibs
-	-m <method> can be used multiple times, but last one gets the dibs
- 	must used either -n or -m BUT can only use one of them
-	if -m <method> is used, and there's multiple dlogs that uses the same <method>,
	the first dlog gets updated. the rest are unchanged.
-	only 1 dlog type is accepted. more than 1 will be ERROR
------------------------------------------------------------------------------------------ */
bool CDlogType::exec()
{
	// if -help, let's do it and exit
	if ( getChild("-help")->has("ok") )
	{
		m_Log << " " << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " L T X                          evx_dlog_type                         L T X" << CUtil::CLog::endl;
		m_Log << "****************************************************************************" << CUtil::CLog::endl;
		m_Log << " " << CUtil::CLog::endl;
		return true;
	}

	CTester& T = CTester::instance();

	// if no options, ERROR: CEX Error: Must specify either a valid method or dlog index.
	if (!m_Args.size())
	{		
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	for (std::list< std::string >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		CArg* p = getChild( *it );

		// this arg does not match any of the valid options for this command
		if (!p)
		{
			m_Log << "CEX Error: Type must be Production or ILQA." << CUtil::CLog::endl;
			return false;
		}

		if (p->is("-n"))
		{
			// if no arg after -n, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": 'end of line' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg is not an integer, ERROR
			if (!CUtil::isInteger( *it ))
			{
				m_Log << "CEX Error: "  << name() << ": '" << (*it) << "' found where 'integer' expected (ltx/tkn)" << CUtil::CLog::endl;
				return false;
			}
			// check if next arg's value is within dlog method's range
			if ( CUtil::toLong( *it ) < 0 || CUtil::toLong( *it ) >= T.ProgCtrl()->getNumDatalogs() )
			{
				m_Log << "CEX Error: valid dlog index is from 0 to " << (T.ProgCtrl()->getNumDatalogs() - 1) << "." << CUtil::CLog::endl;
				return false;
			}	
			// at this point, -n <m> is valid, we save it. 
			p->setValue( *it );
		}
		else if (p->is("-m"))
		{
			// if no arg after -m, ERROR
			if (++it == m_Args.end())
			{
				m_Log << "CEX Error: Must specify a valid dlog method with '" << name() << "'." << CUtil::CLog::endl;
				return false;
			}
			// at this point, -m <method> is valid, we save it. 
			p->setValue( *it );
		}
		// it might be any of the valid dlog type - Production or ILQA
		else
		{
			// if there's more arg after this, ERROR
			if (++it != m_Args.end())
			{
				m_Log << "CEX Error: " << name() << ": Too many arguments. "<< CUtil::CLog::endl;
				return false;
			}
			p->setValue( "ok" ); 
			setValue( *(--it) ); // store here for ease of access
		}						
	}

	// must used either -n or -m BUT can only use one of them
	if ( getChild("-n")->getValue().empty() == getChild("-m")->getValue().empty() )
	{
		m_Log << "CEX Error: Must specify EITHER a valid method OR a dlog index." << CUtil::CLog::endl;
		return false;
	}

	// did we have a valid type?
	if ( !getChild("Production")->has("ok") && !getChild("ILQA")->has("ok") )
	{
		m_Log << "CEX Error: Type must be Production or ILQA." << CUtil::CLog::endl;
		return false;
	}

	// if -n <n> is used
	if ( getChild("-n")->getValue().size() )
	{
		int i = CUtil::toLong( getChild("-n")->getValue() );

		// check if this is valid dlog method
		std::stringstream val;
		val << T.ProgCtrl()->getDatalogString(i, 1, 0);
		if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
 
		if (!val.str().size())
		{
			m_Log << "CEX Error: Dlog index " << i << " is not associated with any methods." << CUtil::CLog::endl;
			return false;
		}
		else 
		{
			T.ProgCtrl()->setDatalogType (i, getValue().c_str() );
			const char* p = T.ProgCtrl()->getDatalogType(i);
			m_Log << "CEX: Type for dlog" << i << " set to " << (p?p:"") << "." << CUtil::CLog::endl;
			return true;
		}
	}
	// if -m <method> is used
	else 
	{
		for (int i = 0; i < T.ProgCtrl()->getNumDatalogs(); i++)
		{
			std::stringstream val;
			val << T.ProgCtrl()->getDatalogString(i, 1, 0);
			if (!val.str().size()) val << T.ProgCtrl()->getDatalogString(i, 2, 0);
			
			if (val.str().compare( getChild("-m")->getValue() ) == 0)
			{
				T.ProgCtrl()->setDatalogType (i, getValue().c_str());
				const char* p = T.ProgCtrl()->getDatalogType(i);
				m_Log << "CEX: Type for method "<<  getChild("-m")->getValue() << " [dlog" << i << "] set to " << (p?p:"") << "." << CUtil::CLog::endl;
				return true;
			}
		}
		m_Log << "CEX Error: The datalogging method " << getChild("-m")->getValue() << " was not found." << CUtil::CLog::endl;
		return false;
	}
	return true;
}


