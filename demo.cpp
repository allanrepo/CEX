#include <sstream>
#include <utility.h>
#include <cex.h>  
 

int main(int argc, char **argv)  
{
	CCex T;
	if (T.scan(argc, argv)) T.exec();

	return 0;
}


