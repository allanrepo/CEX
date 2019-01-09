#ifndef __CMDLINEARGS__
#define __CMDLINEARGS__

#include <iostream>
#include <string>
#include <vector>


class CCmdLineArgs
{
private:
	class CArg
	{
	private:
		std::string m_strOpt;
		std::string m_strDesc;
		std::string m_strParamDesc;
		std::vector< std::string > m_strParams;
	public:
		CArg(const std::string& opt, const std::string& desc, bool bParam = false, const std::string& paramDesc = "");
		virtual ~CArg();

		const std::string& getDesc(){ return m_strDesc; }
		const std::string& getParamDesc(){ return m_strParamDesc; }
		bool is(const std::string& opt)		
		{
			return m_strOpt.compare(opt) == 0? true : false;
		}		

		bool has(const std::string& param)	
		{
			for (unsigned int i = 0; i < m_strParams.size(); i++)
			{
				if (m_strParams[i].compare(param) == 0) return true;
			}
			return false;
		}

		unsigned int count(){ return m_strParams.size(); }
		bool add(const std::string& param);
		const std::string& get(unsigned int nParam);
		const std::string& get(){ return m_strOpt; }

		static const std::string m_strEmpty;

	};
private:
	std::vector< CArg* > m_Args;
	std::vector< CCmdLineArgs::CArg* >::iterator find(const std::string& opt);

public:
	CCmdLineArgs();
	virtual ~CCmdLineArgs();

	// arg list 
	bool scan(int argc, char **argv);
	bool add(const std::string& opt, const std::string& desc, bool bPara, const std::string& paramDes);
	bool display(/*std::ostream& os*/);
	unsigned int count();
	bool has(const std::string& opt);

	// param 
	unsigned int count(const std::string& opt);
	bool has(const std::string& opt, const std::string& param);
	const std::string& get(const std::string& opt, unsigned int nParamIndex = 0);
	bool add(const std::string& opt, const std::string& param);
};


#endif
