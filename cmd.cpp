#include <cmd.h>

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
		listOptMatch(arg, v, true);

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
			listOptMatch(arg, v);

			// is it still ambiguous?
			if (v.size() > 1)
			{			
				m_Log << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
				for (unsigned int i = 0; i < v.size(); i++) m_Log << "'" << v[i]->get() << "', ";
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
					if ( getOpt( getValue() )->getOpt("-help")->has("ok") ) 
						continue;
				}
				// otherwise, let's get the <tester> and enable connect
				v[0]->setValue( *it );
				v[0]->getOpt("connect")->setValue("ok");
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
					T.getOpt("connect")->setValue("");					
				}
				// enable help for this <command>
				getOpt( getValue() )->getOpt("-help")->setValue("ok");
				continue;
			}
		}

		// at this point, it might be the <command> but it might have been a partial match only.
		// <command> must be an exact match. let's test it
		listOptMatch(arg, v);
		
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
		CArg* pCmd = getOpt( getValue() );
		if (pCmd) return pCmd->scan(a);
	}

	// if there's no <command> then we don't do anything. let tester go into loop mode.
	return true;
}

/* ------------------------------------------------------------------------------------------
execute get_head
------------------------------------------------------------------------------------------ */
bool CGetHead::exec()
{
	if ( getOpt("-help")->has("ok") )
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
	}
	else
	{
		m_Log << "CEX: Head number " << CTester::instance().getHead() << CUtil::CLog::endl;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
there should be no options for get_head 
------------------------------------------------------------------------------------------ */
bool CGetHead::scan(std::list< std::string >& Args)
{
	if (Args.size())
	{		
		m_Log << "CEX Error: cex_version: does not accept parameters. Found '" << (*Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	return true;
}

/* ------------------------------------------------------------------------------------------
execute cex_version
------------------------------------------------------------------------------------------ */
bool CCexVersion::exec()
{
	if ( getOpt("-help")->has("ok") )
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
there should be no options for cex_version
------------------------------------------------------------------------------------------ */
bool CCexVersion::scan(std::list< std::string >& Args)
{
	if (Args.size())
	{		
		m_Log << "CEX Error: " << get() << ": Unknown option '" << (*Args.begin()) << "'." << CUtil::CLog::endl;
		return false;
	}
	return true;
}

