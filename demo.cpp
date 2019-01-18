#include <cex.h>
#include <sstream>
#include "CmdLineArgs.h"

// function declarations
void onHandleTesterInput(const std::string&);

int main(int argc, char **argv)
{
	/*
	CArg cex("-cex");
	cex.addValid( CArg("-tester") );
	cex.addValid( CArg("-timeout") );
	cex.addValid( CArg("-help") );
	cex.addValid( CArg("-timeout") );
	cex.addValid( CArg("-timeout") );

	std::cout << cex.getNumValid() << std::endl;
*/

	CCex cex(argc, argv);
	return 0;
}

/* ------------------------------------------------------------------------------------------
handle user input to tester
------------------------------------------------------------------------------------------ */
void onHandleTesterInput(const std::string& strInput)
{
}
