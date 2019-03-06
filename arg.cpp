#include <arg.h>

const std::string CArg::m_strEmpty("");
 
/* ------------------------------------------------------------------------------------------
check if this arg matches the given name, with option for partial match
------------------------------------------------------------------------------------------ */
bool CArg::is(const std::string& name, bool bPartialMatch)
{
	if (!bPartialMatch) return m_strName.compare(name) == 0? true : false; 

	// if returns 0, a match occured on first character of value
	return (m_strName.find(name) == 0)? true : false;
} 

/* ------------------------------------------------------------------------------------------
add a reference option. reference options must have unique values.
------------------------------------------------------------------------------------------ */
bool CArg::addOpt(CArg* pOpt)
{
	// arg must have value that does not exist in list yet before it can be added
	for (unsigned int i = 0; i < m_listOpt.size(); i++)
	{
		if (m_listOpt[i]->m_strName.compare(pOpt->m_strName) == 0) return false;
	}

	// add to end if not yet
	m_listOpt.push_back(pOpt);
	return true;
}

/* ------------------------------------------------------------------------------------------
list all  option found that matches the given name. 
has option search for partial match
------------------------------------------------------------------------------------------ */
void CArg::listOptMatch(const std::string& name, std::vector< CArg* >& v, bool bPartialMatch) const
{
	v.clear();
	for (unsigned int i = 0; i < m_listOpt.size(); i++)
	{
		if (m_listOpt[i]->is(name, bPartialMatch)) v.push_back( m_listOpt[i] );
	}
}

/* ------------------------------------------------------------------------------------------
find option with the given name in the list. if more than one match is found, 
return null. has option to search for partial match, default being exact match
------------------------------------------------------------------------------------------ */
CArg* CArg::getOpt(const std::string& name, bool bPartialMatch) const
{
	std::vector< CArg* > v;
	listOptMatch(name, v, bPartialMatch);
	return v.size() == 1? v[0] : 0;
}





