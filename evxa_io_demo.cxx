//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_io_demo.cxx  --  This program will demonstrate the I/O
//                       capabilities from an external program, e.g., test methods.
//                       This program will ask a question to the user, that
//                       will be displayed in the Dataviewer etc. and the
//                       user's response will be read and printed in this program.
//
//
//
//  In order to compile and link this program, use the evxa_makefile
//  in this directory. You can use the following command:
//
//          make -f evxa_makefile
//
//
//  To run the program, first start the simulator or tester then
//  type the command
//
//           evxa_io_demo -t <tester_name>
//
//
//  If run without any arguments, e.g.,  evxa_io_demo, it will connect to
//  username_sim.
//
//
//  Revision History:
//
//  99-04-20  Shah Alam  Created
//////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>

#include "evxa/TestheadConnection.hxx"


void parseArgs(int argc, char **argv);
void setDefaults();

const int SIZE = 1024;

char testerName[SIZE];
int timeout;
char programName[SIZE];


int main(int argc, char **argv)
    {
    EVXAStatus status;
    
    setDefaults();
    parseArgs(argc, argv);

    
    // Create a connection to a tester.
    TestheadConnection my_th(testerName, 1);
    if (my_th.getStatus() != EVXA::OK)
        {
        fprintf(stdout, "\n");
        fprintf(stderr, "Error in TestheadConnection constructor:\n%s\n",
                my_th.getStatusBuffer());
        exit(1);
        }

    //call the writeln() method to write to the STDOUT stream (dataviewer)
    status = my_th.writeln(STDOUT,"This message should get printed to STDOUT\n");
    if (status != EVXA::OK)
	fprintf(stderr, "Error in TestheadConnection::writeln(): %s\n", my_th.getStatusBuffer());
    
    //call the writeln() method to write to the OCSOUT stream (qterm)
    status = my_th.writeln(OCSOUT,"This message should get printed to OCSOUT\n");
    if (status != EVXA::OK)
	fprintf(stderr, "Error in TestheadConnection::writeln(): %s\n", my_th.getStatusBuffer());

    
    //call the writeln() method to write to the STDOUT and OCSOUT streams
    status = my_th.writeln(STDOUT | OCSOUT,"This message should get printed to both STDOUT and OCSOUT\n");
    if (status != EVXA::OK)
	fprintf(stderr, "Error in TestheadConnection::writeln(): %s\n", my_th.getStatusBuffer());
    

    //call the readln() method and pass in the prompt, size of the reply buffer and
    //the reply buffer.
    
    //first allocate the reply buffer
    char reply[SIZE];
    status = my_th.readln("Please type in a string followed by RETURN.\n",SIZE,reply);
    
    if (status != EVXA::OK)
	fprintf(stderr, "Error in TestheadConnection::readln(): %s\n", my_th.getStatusBuffer());
    else
	fprintf(stdout, "Reply: %s\n",reply);
    
}


void setDefaults()
    {
    if ( getenv("LTX_TESTER")) {
    	strcpy(testerName, getenv("LTX_TESTER"));}
    else {
        uid_t uid = getuid();
        char buf_passw[1024];
        int rc;
        struct passwd password;
        struct passwd *passwd_info;

        rc = getpwuid_r(uid, &password, buf_passw, 1024, &passwd_info);

        // Use the passwd info to set the tester name based on the user name.
        if (!passwd_info)
            {
            if (rc != 0)
                errno = rc;
            strcpy(testerName, "sim");
            }
        else
            {
            strcpy(testerName, passwd_info->pw_name);
            strcat(testerName, "_sim");
            }
        }

    timeout = 20;
    strcpy(programName, "");

    return;
    }


void parseArgs(int argc, char **argv)
    {
    // If there are any arguments, parse them.
    for (int i = 1; i < argc; i++)
        {
        if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-tester"))
            {
            if ( ++i >= argc )
                {
                fprintf(stderr, "ERROR: found -t/-tester argument but no tester name.\n");
                fprintf(stderr, "       Using tester: %s\n", testerName);
                }
            else
                {
                strcpy(testerName, argv[i]);
                }
            }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help"))
            {
            fprintf(stderr, "\nUsage: %s [-tester testerName] [-help]\n\n",
                    argv[0]);
            exit(0);
            }
        else
            {
            fprintf(stderr, "ERROR: found unknown argument '%s'.\n", argv[i]);
            fprintf(stderr, "       Ignoring.\n");
            }
        }

    return;
    }
