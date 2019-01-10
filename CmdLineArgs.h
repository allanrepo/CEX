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
		bool m_enable;
		std::string m_strOpt;
		std::string m_strDesc;
		std::string m_strParamDesc;
		std::vector< std::string > m_strParams;
	public:
		CArg(const std::string& opt, const std::string& desc, bool bParam = false, const std::string& paramDesc = "");
		virtual ~CArg();

		const std::string& getDesc(){ return m_strDesc; }
		const std::string& getParamDesc(){ return m_strParamDesc; }
		bool is(const std::string& opt);
		bool has(const std::string& param);
		unsigned long size(){ return m_strParams.size(); }
		bool add(const std::string& param);
		const std::string& get(unsigned int nParam);
		const std::string& get(){ return m_strOpt; }
		bool enable(bool s = true){ m_enable = s; return s; }
		bool enabled(){ return m_enable; }

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
	bool add(const std::string& opt, bool bPara, const std::string& desc = "", const std::string& paramDesc = "");
	bool display(/*std::ostream& os*/);
	unsigned long size();
	bool has(const std::string& opt);

	// param 
	unsigned long size(const std::string& opt);
	bool has(const std::string& opt, const std::string& param);
	const std::string& get(const std::string& opt, unsigned int nParamIndex = 0);
	bool add(const std::string& opt, const std::string& param);
	long getAsLong(const std::string& opt, unsigned int nParamIndex = 0);
	bool enabled(const std::string& opt);
	
};


#endif
