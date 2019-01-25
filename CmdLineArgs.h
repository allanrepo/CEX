#ifndef __CMDLINEARGS__
#define __CMDLINEARGS__

#include <iostream>
#include <string>
#include <vector>

/*
rules:
- can't have multiple -command


- can have -t <tester> after -c load <test program>
- can have -t <tester> after -c unload

-t <tester>
	- can only have 1 parameter
	- can call other arguments before or after it

-help
	- if called before or after -c <command>
		- it prints the help information about <command> 
		- do nothing (<command> is not executed, does not connect to tester)
		- any violation after -c <command> is ignored
		- any violation before -c <command> is reported
	- if called without -c command
		- it prints command list 
		- do nothing (does not connect to tester)

-help
	- no -c <command>
		- prints general help

	- before the -c <command>
		- if -t <tester> anywhere
			- connect <tester>
			- prints <command> help
		- else 
			- ERROR: can't connect to tester
	
	- after the -c <command>
		- prints <command> help

*/


class CCmdLineArgs
{
public:
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
		bool is(const std::string& opt, bool bExactMatch = false);
		bool has(const std::string& param);
		unsigned long size(){ return m_strParams.size(); }
		bool add(const std::string& param);
		bool set(const std::string& param);
		const std::string& get(unsigned int nParam);
		const std::string& get(){ return m_strOpt; }
		bool enable(bool s = true){ m_enable = s; return s; }
		bool enabled(){ return m_enable; }

		static const std::string m_strEmpty;

	};
private:
	std::vector< CArg* > m_Args;
	std::vector< CCmdLineArgs::CArg* >::iterator find(const std::string& opt, bool bExactMatch = false);

	
	

public:
	CCmdLineArgs();
	virtual ~CCmdLineArgs();

	// arg list 
	bool scan(int argc, char **argv);
	bool add(const std::string& opt, bool bPara, const std::string& desc = "", const std::string& paramDesc = "");
	bool display(/*std::ostream& os*/);
	unsigned long size();
	bool has(const std::string& opt);
	bool ambiguous(const std::string& opt);
	void get(const std::string& opt, std::vector< std::string >& v);

	// param 
	unsigned long size(const std::string& opt);
	bool has(const std::string& opt, const std::string& param);
	const std::string& get(const std::string& opt, unsigned int nParamIndex = 0);
	bool add(const std::string& opt, const std::string& param);
	bool set(const std::string& opt, const std::string& param);
	long getAsLong(const std::string& opt, unsigned int nParamIndex = 0);
	bool enabled(const std::string& opt);
	
};



class CArg
{
private:
	std::string m_strValue;
	std::vector< CArg* > m_listValid;
	std::vector< std::string > m_listParam;

public:
	CArg(const std::string strValue = "");
	virtual ~CArg();

	// get the string value
	const std::string& get() const { return m_strValue; }

	CArg* get(const std::string& arg, bool bExactMatch = false) const 
	{
		std::vector< CArg* > v;
		listValidMatch(arg, v, bExactMatch);
		return v.size() == 1? v[0] : 0;
	}	


	// check if this arg matches the given value, either exact match or partial
	bool is(const CArg& arg, bool bExactMatch = false);	

	// get list of args that matches the given value
	void listValidMatch(const CArg& arg, std::vector< CArg* >& v, bool bExactMatch = false) const;

	// add to list of valid arguments. if an arg with same value exists, does not get added instead
	bool addValid(CArg* valid);

	unsigned int getNumValid(){ return m_listValid.size(); }

	// add a param for this arg
	bool addParam(const std::string& param);

	// returns the param of specified index. if index is invalid, returns empty string
	const std::string& getParam(unsigned int i = 0) const;

	void clearParam(){ m_listParam.clear(); }
	unsigned int getNumParam() const { return m_listParam.size(); }
	bool hasParam(const std::string& param) const;

	static const std::string m_strEmpty;







};




#endif
