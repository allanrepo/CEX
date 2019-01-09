#include "CmdLineArgs.h"

const std::string CCmdLineArgs::CArg::m_strEmpty("");






/* ------------------------------------------------------------------------------------------
arg object's constructor
------------------------------------------------------------------------------------------ */
CCmdLineArgs::CArg::CArg(const std::string& opt, const std::string& desc, bool bParam, const std::string& paramDesc)
{
	m_strOpt = opt;
	m_strDesc = desc;
	m_strParamDesc = paramDesc;

}

CCmdLineArgs::CArg::~CArg()
{
}

/* ------------------------------------------------------------------------------------------
this function lets you add param to an arg object.
arg object has a container that stores a list of param. this inserts it to the end
it also ensures the param to be added does not exist in the list before it adds it
making sure there's no duplicate
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::CArg::add(const std::string& param)
{
	// check if this param already exist in container
	for (unsigned int i = 0; i < m_strParams.size(); i++)
	{
		if (m_strParams[i].compare(param) == 0) return false;
	}

	// add to end if not yet
	m_strParams.push_back(param);
	return true;
}

/* ------------------------------------------------------------------------------------------
this function lets you add param to an arg object.
arg object has a container that stores a list of param. this inserts it to the end
it also ensures the param to be added does not exist in the list before it adds it
making sure there's no duplicate
------------------------------------------------------------------------------------------ */
const std::string& CCmdLineArgs::CArg::get(unsigned int nParam)
{
	if (nParam >= m_strParams.size()) return m_strEmpty;
	else return m_strParams[nParam];
}



/* ------------------------------------------------------------------------------------------
search the arg list if there's any that contains this opt. return reference to its 
iterator if found. otherwise, just return reference to list end
------------------------------------------------------------------------------------------ */
std::vector< CCmdLineArgs::CArg* >::iterator CCmdLineArgs::find(const std::string& opt)
{
	// loop through args 
	for (std::vector< CArg* >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		if ( (*it)->is(opt) ) return it;		
	}

	return m_Args.end();
}

/* ------------------------------------------------------------------------------------------
class constructor/destructor
------------------------------------------------------------------------------------------ */
CCmdLineArgs::CCmdLineArgs()
{
}

CCmdLineArgs::~CCmdLineArgs()
{
}

/* ------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::add(const std::string& opt, const std::string& param)
{
	// find the opt in the list, if found, add this param
	std::vector< CArg* >::iterator it = find(opt);

	if (it != m_Args.end())
	{
		return (*it)->add(param);
	}
	else return false;

}


/* ------------------------------------------------------------------------------------------
scan through the arguments passed and check if they in our argument list.
if they are, check if parameter is being passed to them and add them
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::scan(int argc, char **argv)
{
	// quick exit if invalid args
	if (argc == 0 || argv == NULL) return false;

	// loop through args, start at 1 to skip to arguments already
	char* p = 0;
	std::vector< CArg* >::iterator it = m_Args.end();
	for (int i = 1; i < argc; i++) 
	{
		// arg option can have '-' or '--' prefix, we accept both
		if (argv[i][0] == '-')
		{ 
			p = argv[i];
			argv[i][1] == '-' ? p+= 2 : p+= 1;
			// excluding the prefix, find this arg from our list and get reference for use later
			it = find(p);
		} 			
		// or is it param?
		else
		{
			// check if this param belongs to an arg that we already have in our list. 
			// if yes, add it. the add() function ensures this param does not exist in its list before adding.
			if (it != m_Args.end())(*it)->add(argv[i]);
		}	
	}

	return true;
}

/* ------------------------------------------------------------------------------------------
define a new arg option 
desc and paramDesc is primarily used only for display help
note that if this opt already exists somewhere in the list, this will not
be added
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::add(const std::string& opt, const std::string& desc, bool bParam, const std::string& paramDesc)
{
	// search if this opt already exist in our list
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		if (m_Args[i]->is(opt)) return false;
	}
	// create new arg object containing this and add to our list
	m_Args.push_back(new CCmdLineArgs::CArg(opt, desc, bParam, paramDesc));
	return true;
}


/* ------------------------------------------------------------------------------------------
returns the number of args 
------------------------------------------------------------------------------------------ */
unsigned int CCmdLineArgs::count()
{
	return m_Args.size();
}

/* ------------------------------------------------------------------------------------------
returns the number of parameter this arg has 
------------------------------------------------------------------------------------------ */
unsigned int CCmdLineArgs::count(const std::string& opt)
{
	// find this opt (long or short) in our arg list
	std::vector< CCmdLineArgs::CArg* >::iterator it = find(opt);

	// if found, return its param count, otherwise return 0
	if (it != m_Args.end()) return (*it)->count();
	else return 0;
}

/* ------------------------------------------------------------------------------------------
check if we have this arg option in our arg list
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::has(const std::string& opt)
{
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		if (m_Args[i]->is(opt) ) return true;
	}
	return false;
}

/* ------------------------------------------------------------------------------------------
check if this param exist in this arg option
------------------------------------------------------------------------------------------ */
const std::string& CCmdLineArgs::get(const std::string& opt, unsigned int nParam)
{
	std::vector< CArg* >::iterator it = find(opt);

	if (it != m_Args.end()) return (*it)->get(nParam);
	else return CCmdLineArgs::CArg::m_strEmpty;
}

 


bool CCmdLineArgs::display(/*std::ostream& os*/)
{
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		std::cout << m_Args[i]->get() << " - ";
		for (unsigned int j = 0; j < m_Args[i]->count(); j++) 
		{
			std::cout << m_Args[i]->get(j) << ", ";
		}
		std::cout << std::endl;
	}

	return true;
}






#if 0

/* ------------------------------------------------------------------------------------------
arg object's constructor
------------------------------------------------------------------------------------------ */
CArgs::CArg::CArg(const std::string& opt, const std::string& desc, bool bParam, const std::string& paramDesc)
{
	m_strOpt = opt;
	m_strDesc = desc;
	m_strParamDesc = paramDesc;

}

CArgs::CArg::~CArg()
{
}

/* ------------------------------------------------------------------------------------------
this function lets you add param to an arg object.
arg object has a container that stores a list of param. this inserts it to the end
it also ensures the param to be added does not exist in the list before it adds it
making sure there's no duplicate
------------------------------------------------------------------------------------------ */
bool CArgs::CArg::add(const std::string& param)
{
	// check if this param already exist in container
	for (unsigned int i = 0; i < m_strParams.size(); i++)
	{
		if (m_strParams[i].compare(param) == 0) return false;
	}

	// add to end if not yet
	m_strParams.push_back(param);
	return true;
}

/* ------------------------------------------------------------------------------------------
this function lets you add param to an arg object.
arg object has a container that stores a list of param. this inserts it to the end
it also ensures the param to be added does not exist in the list before it adds it
making sure there's no duplicate
------------------------------------------------------------------------------------------ */
bool CArgs::CArg::addOption(const std::string& opt)
{
	// check if this param already exist in container
	for (unsigned int i = 0; i < m_strOpts.size(); i++)
	{
		if (m_strOpts[i].compare(opt) == 0) return false;
	}

	// add to end if not yet
	m_strOpts.push_back(opt);
	return true;
}
#endif


