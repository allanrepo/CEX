#include <cex.h>
#include <sstream>
#include "CmdLineArgs.h"

// function declarations
void onHandleTesterInput(const std::string&);

int main(int argc, char **argv)
{

	CCex cex(argc, argv);
	//if (cex.connect())
	{
	//	cex.loop(); 
	}

	return 0;

}

/* ------------------------------------------------------------------------------------------
handle user input to tester
------------------------------------------------------------------------------------------ */
void onHandleTesterInput(const std::string& strInput)
{
}
