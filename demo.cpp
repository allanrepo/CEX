#include <sstream>
#include <utility.h>
#include <cex.h>  
#include <cmd.h>
#include <xml.h> 

int main(int argc, char **argv)  
{
#if 0
	CXml *pXml = new CXml("help.xml");
	std::cout << pXml->fetchTag() << std::endl;

	for (int i = 0; i < pXml->numChildren(); i++)
	{
		CXml* pNode = pXml->fetchChild(i);
		std::cout << "    " << pNode->fetchTag();
		std::cout << (pNode->fetchText().size()? " - ": "") << "--" << pNode->fetchText() << "--" << std::endl;
	}
	if (pXml) delete pXml;
	return 0;
#endif

	CCex T;

	// add commands here
	T.addCmd(new CGetHead());
	T.addCmd(new CCexVersion());
	T.addCmd(new CGetName());
	T.addCmd(new CLoad());
	T.addCmd(new CUnload());
	T.addCmd(new CStart());
	T.addCmd(new CGetUserName());
	T.addCmd(new CProgramLoaded());
	T.addCmd(new CProgramLoadDone());
	T.addCmd(new CGetExp());
	T.addCmd(new CEvxSummary());
	T.addCmd(new CDlogMethods());
	T.addCmd(new CDlogFileFreq());

	// parse command line args and execute 
	if (T.scan(argc, argv)) T.exec();

	return 0;
}


