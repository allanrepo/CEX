//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_basic_demo.cxx  --  This program will demonstrate the basic
//                          features of the EVXA library, performing
//                          useful operations using the different classes
//                          that are provided in the library.
//
//
//
//  In order to compile and link this program, use the evxa_makefile
//  in this directory. You can use the following command:
//
//          make -f evxa_makefile
//
//  To run the program, first start the simulator or tester then
//  type the command
//
//           evxa_basic_demo -t <tester_name>
//
//
//  If run without any arguments, e.g.,  evxa_basic_demo, it will connect to
//  username_sim.
//
//
//  Revision History:
//
//  99-07-12  Ben Isherwood  Added more functions to be printed 
//  99-04-12  Shah Alam  Created
//////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <cstdio>

#include "evxa/ProgramControl.hxx"
#include "alarms_def.hxx"


void parseArgs(int argc, char **argv);
void setDefaults();
int timeSinceLoaded ( ProgramControl *prog_ctrl, int& days, int& hours, int& minutes, double& seconds );

const int SIZE = 1024;

char testerName[SIZE];
int timeout;
char programName[SIZE];


int main(int argc, char **argv)
    {
    int days=0, hours=0, minutes=0;
    double seconds=0;
            
    setDefaults();
    parseArgs(argc, argv);

    // Create a connection to a tester, and a ProgramControl.
    TestheadConnection my_testhead(testerName, 1, timeout);
    ProgramControl myProgramSession(my_testhead);
    if (myProgramSession.getStatus() != EVXA::OK)
        {
        fprintf(stdout, "\n");
        fprintf(stderr, "Error in ProgramControl constructor:\n%s\n",
                myProgramSession.getStatusBuffer());
        exit(1);
        }
    
    fprintf(stdout, "\n");
    fprintf(stdout, "Tester: %s\n", my_testhead.getName());    

    // Print out the software and tester revisions.
    const int getVNum = 1;
    fprintf(stdout, "\n");
    fprintf(stdout, "Software Revision:\t%s\n", EVXA::getRevision(getVNum));
    fprintf(stdout, "Tester Revision:\t%s\n", my_testhead.getTesterRevision(getVNum));

    // Get the TC Status.
    fprintf(stdout, "\n");
    fprintf(stdout, "Tester Status:\n");
    fprintf(stdout, "----------\n");
    fprintf(stdout, "Head Power:      %s\n", my_testhead.isHeadPowered() ? "Yes" : "No");
    fprintf(stdout, "Program Loaded:  %s\n", my_testhead.isProgramLoaded() ? "Yes" : "No");
    fprintf(stdout, "Program Running: %s\n", my_testhead.isProgramRunning() ? "Yes" : "No");
    fprintf(stdout, "Tester Ready:    %s\n", my_testhead.isTesterReady() ? "Yes" : "No");

    if (myProgramSession.isProgramLoaded()) {
        // Get Program Name
        fprintf(stdout, "\n");
        fprintf(stdout, "Program Name:\n");
        fprintf(stdout, "----------\n");
        fprintf(stdout, "%s\n", myProgramSession.getProgramName());

        // Get Program Path
        fprintf(stdout, "\n");
        fprintf(stdout, "Program Path:\n");
        fprintf(stdout, "----------\n");
        fprintf(stdout, "%s\n", myProgramSession.getProgramPath());

        // Get Username
        fprintf(stdout, "\n");
        fprintf(stdout, "User Name:\n");
        fprintf(stdout, "----------\n");
        fprintf(stdout, "%s\n", myProgramSession.getUserName());

        // Get Display
        fprintf(stdout, "\n");
        fprintf(stdout, "Display:\n");
        fprintf(stdout, "----------\n");
        fprintf(stdout, "%s\n", myProgramSession.getDisplay());

        if ((timeSinceLoaded ( &myProgramSession, days, hours, minutes, seconds )) == 0) {
            fprintf(stdout, "\n");
            fprintf(stdout, "Test program has been loaded:\n");
            fprintf(stdout, "%d days, %d hours, %d minutes, and %f seconds\n", days, hours, minutes, seconds);
        }
    }
    
    exit(0);
}


void setDefaults()
    {
    if ( getenv("LTX_TESTER")) {
    	strncpy(testerName, getenv("LTX_TESTER"), SIZE);}
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

int timeSinceLoaded ( ProgramControl *prog_ctrl, int& days, int& hours, int& minutes, double& seconds )
    {
    const int MINUTES = 60, HOURS = 60*MINUTES, DAYS = 24*HOURS;
    double time_loaded=0, time_now=0, duration=0;
    struct timeval time_info;
    
    time_loaded = atof(prog_ctrl->getStartTime());
    // get the current time the same way getStartTime does, using gettimeofday()
    if (gettimeofday(&time_info, NULL) == 0) {
        time_now = (double) time_info.tv_sec + ((double) time_info.tv_usec * 1e-6);
        duration = time_now - time_loaded;  // time in seconds since loaded
        days = (int)duration/DAYS;
        hours = ((int)duration % DAYS)/HOURS;
        minutes = ((int)duration % HOURS)/MINUTES;
        seconds = duration - double(days*DAYS + hours*HOURS + minutes*MINUTES);
        return (0);
        }
    else {
        fprintf(stderr, "\nERROR: Unable to get current time using 'gettimeofday()'\n");
        return (1);
        } 
    }
