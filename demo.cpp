#include <sstream>
#include <utility.h>
#include <cex.h>  
#include <cmd.h>
 

int main(int argc, char **argv)  
{
	CCex T;

	// add commands here
	T.addCmd(new CGetHead());
	T.addCmd(new CCexVersion());
	T.addCmd(new CGetName());
	T.addCmd(new CLoad());
	T.addCmd(new CUnload());
	T.addCmd(new CStart());
	T.addCmd(new CGetUserName());

	// parse command line args and execute 
	if (T.scan(argc, argv)) T.exec();

	return 0;
}


