#include <cex.h>
#include <cmd.h>


/* ------------------------------------------------------------------------------------------
constructor 
------------------------------------------------------------------------------------------ */
CCex::CCex(): m_Tester(CTester::instance()), m_Log(CTester::instance().m_Log), m_Debug(CTester::instance().m_Debug)
{  
	// debug log is immediate and disabled by default 
	m_Debug.immediate = true;
	m_Debug.enable = false;
	m_Debug.silent = true;
  
	// logger is always enabled and immediate 
	m_Log.immediate = true;
	m_Log.enable = true;    
	m_Log.silent = false; 
 
	// create -t[ester] option
	addOpt( &CTester::instance() );

	// create -h[elp] option
	CArg* pHelp = new CHelp();
	addOpt( pHelp );

	// create -c[ommand] option
	CArg* pCmd = new CCmd(); 
	pCmd->addOpt(new CGetHead());
	pCmd->addOpt(new CCexVersion());
	pCmd->addOpt(new CGetName());
	pCmd->addOpt(new CLoad());
	addOpt( pCmd );

	// add options 
	addOpt( new CArg("-debug") );
	addOpt( new CArg("-dm") );
	addOpt( new CArg("-timeout") );
	addOpt( new CArg("-head") );
	addOpt( new CArg("-hd") );
	addOpt( new CArg("-syntax_check") );
	addOpt( new CArg("-version") );	
	
	// let's initialize <tester> to default name
	std::stringstream ss;
	getUserName().empty()? (ss << "sim") : (ss << getUserName() << "_sim");
	getOpt("-tester")->setValue(ss.str()); 
	
	// let's set tester connection enabled by default 
	getOpt("-tester")->getOpt("connect")->setValue("ok");
}

/* ------------------------------------------------------------------------------------------
get user name
------------------------------------------------------------------------------------------ */
const std::string CCex::getUserName() const
{ 
	uid_t uid = getuid();
	char buf_passw[1024];    
	struct passwd password;
	struct passwd *passwd_info;

	getpwuid_r(uid, &password, buf_passw, 1024, &passwd_info);
	return std::string(passwd_info->pw_name);
}

/* ------------------------------------------------------------------------------------------
scan command line args
------------------------------------------------------------------------------------------ */
bool CCex::scan(int argc, char **argv) 
{
	// store all args to list
	std::list< std::string > v;
	for (int i = 1; i < argc; i++) v.push_back( argv[i] );

	// parse command line arguments
	if (!scan(v)) return false;

	return true;
}

/* ------------------------------------------------------------------------------------------
scan command line arguments
------------------------------------------------------------------------------------------ */
bool CCex::scan(std::list< std::string >& Args)
{
	for (std::list< std::string >::iterator it = Args.begin(); it != Args.end(); it++)
	{ 
		std::string arg( (*it) );

		// handle '-t' exception. it's a special case as per unison doc, in which it always refer 
		// to '-tester' even if it's ambiguous to other opts such as '-timeout'.
		if (arg.compare("-t") == 0) arg = "-tester"; 	

		// list all options that partially match this arg
		std::vector< CArg* > v;
		listOptMatch(arg, v, true);

		// is it ambiguous?
		if (v.size() > 1)
		{
			m_Log << "CEX Error: CEX arguments: Ambiguous option '" << arg << "' choices are: ";
			for (unsigned int i = 0; i < v.size(); i++) m_Log << "'" << v[i]->get() << "', ";
			m_Log << CUtil::CLog::endl;
			return false;
		}   
  
		// did we not find a match?
		if (!v.size())
		{ 
			m_Log << "CEX Error: CEX arguments: Bad argument to cex '" << arg << "'" << CUtil::CLog::endl;
			return false;
		}

		// or did we find a unique match?

		// is it -t[ester]?
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
				v[0]->setValue( *it );
				v[0]->getOpt("connect")->setValue("ok");
				continue;
			}
		}

		if (v[0]->is("-help"))
		{
			v[0]->setValue("ok");
			continue;
		}		

		if (v[0]->is("-debug"))
		{
			v[0]->setValue("ok");
			continue;
		}

		// is it -c[ommand]?
		if (v[0]->is("-command"))
		{
			std::list< std::string > c;
			c.splice(c.begin(), Args, ++it, Args.end() );
			return v[0]->scan(c);			
		}	
	}

	return true;
}


/* ------------------------------------------------------------------------------------------
 
------------------------------------------------------------------------------------------ */
bool CCex::exec()
{
	// are we in debug mode?
	if (getOpt("-debug")->has("ok"))
	{
		m_Debug.enable = true;
		m_Debug.silent = false;
	}

	// if general help is enabled, print it and do nothing
	if (getOpt("-help")->has("ok"))
	{
		getOpt("-help")->exec();
		return true;
	}

	// try to connect to tester
	if (getOpt("-tester")->getOpt("connect")->has("ok") )
	{
		if (!m_Tester.connect( getOpt("-tester")->getValue() )) return false;
	}

	// are we executing command?
	if (!getOpt("-command")->getValue().empty())
	{
		std::string cmd = getOpt("-command")->getValue();
		CArg* pCmd = getOpt("-command")->getOpt( cmd );
		if (pCmd)
		{
			pCmd->exec();
		}
		else
		{
			m_Tester.loop();
		}
	}
	// if not, let's get into loop
	else
	{
		m_Tester.loop();
	}

	m_Tester.disconnect();
	return true;
}



