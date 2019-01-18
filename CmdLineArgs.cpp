#include "CmdLineArgs.h"
#include "utility.h"

const std::string CCmdLineArgs::CArg::m_strEmpty("");






/* ------------------------------------------------------------------------------------------
arg object's constructor
------------------------------------------------------------------------------------------ */
CCmdLineArgs::CArg::CArg(const std::string& opt, const std::string& desc, bool bParam, const std::string& paramDesc)
{
	m_strOpt = opt;
	m_strDesc = desc;
	m_strParamDesc = paramDesc;
	m_enable = false;
}

CCmdLineArgs::CArg::~CArg()
{
}

/* ------------------------------------------------------------------------------------------
check if this arg has this param
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::CArg::has(const std::string& param)	
{
	for (unsigned int i = 0; i < m_strParams.size(); i++)
	{
		if (m_strParams[i].compare(param) == 0) return true;
	}
	return false;
}


/* ------------------------------------------------------------------------------------------
check if this is the arg we are looking for. either exact match or pattern match

------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::CArg::is(const std::string& opt, bool bExactMatch)
{ 
	if (bExactMatch) return m_strOpt.compare(opt) == 0? true : false; 

	// check if this opt matches the string 'opt' argument we looking for
	size_t t = m_strOpt.find(opt);

	// if there's a match, check if return value = 0. we expect to match the 
	// first set of characters e.g. -co matches -command, but -omm does not.
	if (t == 0) return true;
	else return false;
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
this function lets you set a param to an arg object.
any existing param is removed and replaced by this single param
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::CArg::set(const std::string& param)
{
	m_strParams.clear();
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
search the arg list for the first one that matches the given opt at first character level.
e.g. -te matches 'tester' but -este does not. also, if there's ambiguous matches, e.g. -t
matches 'tester' and 'timeout', the first one in the list get selected. 
------------------------------------------------------------------------------------------ */
std::vector< CCmdLineArgs::CArg* >::iterator CCmdLineArgs::find(const std::string& opt, bool bExactMatch)
{
	// loop through args and find the first match
	for (std::vector< CArg* >::iterator it = m_Args.begin(); it != m_Args.end(); it++)
	{
		if ( (*it)->is(opt) ){ return it; }
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
find the opt in list and add the param to it. if param already exists in list, it does
nothing. otherwise it adds to end of the list
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
find the opt in list and set param to it. it removes any existing param in the opt
and add this new one
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::set(const std::string& opt, const std::string& param)
{
	// find the opt in the list, if found, add this param
	std::vector< CArg* >::iterator it = find(opt);

	if (it != m_Args.end())
	{
		return (*it)->set(param);
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
		// arg can be option or param. if it has '-' prefix, it's an option
		if (argv[i][0] == '-')
		{ 
			p = argv[i];
			argv[i][1] == '-' ? p+= 2 : p+= 1;
			// excluding the prefix, find this arg from our list and get reference for use later
			it = find(p);
			
			// if this arg option is in our list, enable it so we know that it is active
			if (it != m_Args.end())(*it)->enable();
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
bool CCmdLineArgs::add(const std::string& opt, bool bParam, const std::string& desc, const std::string& paramDesc)
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
unsigned long CCmdLineArgs::size()
{
	return m_Args.size();
}

/* ------------------------------------------------------------------------------------------
returns the number of parameter this arg has 
------------------------------------------------------------------------------------------ */
unsigned long CCmdLineArgs::size(const std::string& opt)
{
	// find this opt (long or short) in our arg list
	std::vector< CCmdLineArgs::CArg* >::iterator it = find(opt);

	// if found, return its param size, otherwise return 0
	if (it != m_Args.end()) return (*it)->size();
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

------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::ambiguous(const std::string& opt)
{
	unsigned long n = 0;
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		if (m_Args[i]->is(opt) ) n++;
	}
	if (n>1) return false;
	else return true;
} 

/* ------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------ */
void CCmdLineArgs::get(const std::string& opt, std::vector< std::string >& v)
{
	v.clear();
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		if (m_Args[i]->is(opt) ) v.push_back( m_Args[i]->get() );
	}
}

/* ------------------------------------------------------------------------------------------
get the param value with the given option and index
------------------------------------------------------------------------------------------ */
const std::string& CCmdLineArgs::get(const std::string& opt, unsigned int nParam)
{
	std::vector< CArg* >::iterator it = find(opt);

	if (it != m_Args.end()) return (*it)->get(nParam);
	else return CCmdLineArgs::CArg::m_strEmpty;
}

/* ------------------------------------------------------------------------------------------
get the param value with the given option and index - as long
------------------------------------------------------------------------------------------ */
long CCmdLineArgs::getAsLong(const std::string& opt, unsigned int nParam)
{
	std::string s = get(opt, nParam);
	if (s.compare(CCmdLineArgs::CArg::m_strEmpty) == 0) return 0;
	else return toLong(s);
}

/* ------------------------------------------------------------------------------------------
check if this option is in the scan list of arguments called during execution
------------------------------------------------------------------------------------------ */
bool CCmdLineArgs::enabled(const std::string& opt)
{
	std::vector< CArg* >::iterator it = find(opt);

	if (it != m_Args.end()) return (*it)->enabled();	
}


bool CCmdLineArgs::display(/*std::ostream& os*/)
{
	for (unsigned int i = 0; i < m_Args.size(); i++)
	{
		std::cout << m_Args[i]->get() << " - ";
		for (unsigned int j = 0; j < m_Args[i]->size(); j++) 
		{
			std::cout << m_Args[i]->get(j) << ", ";
		}
		std::cout << std::endl;
	}

	return true;
}



const std::string CArg::m_strEmpty("");
const CArg CArg::m_ArgEmpty("");

/* ------------------------------------------------------------------------------------------
constructor/desctructor
------------------------------------------------------------------------------------------ */
CArg::CArg(const std::string strValue)
{
	m_strValue = strValue;
}

CArg::~CArg()
{
}

/* ------------------------------------------------------------------------------------------
check if the given arg matches this arg. match can either be exact match or partial 
------------------------------------------------------------------------------------------ */
bool CArg::is(const CArg& arg, bool bExactMatch)
{
	if (bExactMatch) return m_strValue.compare(arg.m_strValue) == 0? true : false;

	// find arg as part of value
	size_t t = m_strValue.find(arg.m_strValue);

	// if returns 0, a match occured on first character of value
	if (t == 0) return true;
	else return false;
}

/* ------------------------------------------------------------------------------------------
add a valid param for reference. ensures the value is unique. if there's any arg in list
that has same value as the arg to be added, the arg is not added anymore
------------------------------------------------------------------------------------------ */
bool CArg::addValid(const CArg& valid)
{
	// arg must have value that does not exist in list yet before it can be added
	for (unsigned int i = 0; i < m_listValid.size(); i++)
	{
		if (m_listValid[i].m_strValue.compare(valid.m_strValue) == 0) 
			return false;
	}

	// add to end if not yet
	m_listValid.push_back(valid);
	return true;
}


/* ------------------------------------------------------------------------------------------
find valid args that partialy matches the given arg. 
------------------------------------------------------------------------------------------ */
void CArg::listValidMatch(const CArg& arg, std::vector< CArg >& v)
{
	v.clear();
	for (unsigned int i = 0; i < m_listValid.size(); i++)
	{
		if (m_listValid[i].is(arg)) v.push_back( m_listValid[i] );
	}
}

/* ------------------------------------------------------------------------------------------
add param to list
------------------------------------------------------------------------------------------ */
bool CArg::addParam(const std::string& param)
{
	m_listParam.push_back(param);
}

/* ------------------------------------------------------------------------------------------
get the param for corresponding index. returns empty string if index is invalid
------------------------------------------------------------------------------------------ */
const std::string& CArg::getParam(unsigned int i) const
{
	if (i >= m_listParam.size()) return m_strEmpty;
	else return m_listParam[i];	
} 



/* ------------------------------------------------------------------------------------------
add arg that was scanned. it makes sure first it's valid
------------------------------------------------------------------------------------------ */
bool addScanArg(const CArg& scanArg)
{
	// check if this is valid

	// add
	return true;
}


bool CArg::isValid(const std::string& arg, bool bExactMatch)
{
//	if (bExactMatch)
	return true;
}


/* ------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------ */
std::vector< CArg >::iterator CArg::find(const std::string& arg, bool bExactMatch)
{
	return m_listValid.end();
}



