#include <cex.h>
#include "argSwitches.h"

int main(int argc, char **argv)
{
	utilities::args::argSwitches args;

        args.addArg("t", "tester", "Specifies the target tester, if not set, the environment variable LTX_TESTER will be checked, followed by <username>_sim", true, "tester name");
        args.addArg("d", "debug", "Enable debug logic", false, "debug");
        args.addArg("v", "version", "Display the version and exit", false, "vversion");
       
	args.scanArgs(argc, argv);

	std::string ss = args.getArg("tester"); 
	std::cout << "tester: " << ss << std::endl;

	// if longopt flag = true (default), only longopt is check for state
	bool debug = args.isSet("debug");
	std::cout << "debug: " << (debug? "yes" : "no") << std::endl;

	// if longopt flag = false, only shortopt is check for state
	bool version = args.isSet("v", false);
	std::cout << "version: " << (version? "yes" : "no") << std::endl;

	args.displayHelp(std::cout);

}
