//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_program_demo.cxx  --  This program will demonstrate the basic
//                     features of the EVXA library, performing
//                     useful operations using the different classes
//                     that are provided in the library.
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
//           evxa_program_demo -tester <tester_name> -head <head_num>
//
//   If run without any arguments, e.g.,  evxa_program_demo, it will connect to
//   username_sim and head 1.
//   
//
//  The program waits for user input on stdin and the executes that input.
//  If another type of interface is used (ie GUI) then the select loop
//  in this file will need to be modified to handle requests from that 
//  interface..  
//
//  Type "quit" to exit the program.
//
//
//  This file contains main and the functions necessary to connect to the
//  to the tester.  The select() loop is handled in this file also.
//
//
//  Below is a brief description of what the remaining files do for this
//  program example.
//
//  evxa_program_state_notification.cxx handles the system notifications. 
//  
//  evxa_program_entry_points.cxx is where the user implements code for
//  the system notifications.  This file will grow to be user dependent.
//
//  evxa_program_input.cxx is where the user implements his interface.
//  In this example stdin is used as the interface.  The commands are
//  read from stdin and executed. 
//
//  Revision History: 
//
//  99-05-11  Jim Panopulos  Created
//////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#include <evxa_program_demo.hxx>

//-----------------------------------------------------------
extern void commandHandler();
extern void handleStdIn();
extern void executeHelp();

void parseArgs(int argc, char **argv);
void setDefaults();

//External Globals ------------------------------------------
TesterObjectsStruct TesterObjects;
int exitServerLoop = 0;  //Flag to exit server loop and program
int testerReconnect = 0; //Flag to reconnect to tester.

const int SIZE = 1024;

char testerName[SIZE];
int head;
char programName[SIZE];


//---- TestObjects class functions ---------------------------
TesterObjectsStruct::TesterObjectsStruct() : Tester(NULL), cxnPtr(NULL),
  Program(NULL), statePtr(NULL), evxioPtr(NULL)
{
}

TesterObjectsStruct::~TesterObjectsStruct()
{
  deleteTesterObjects();
}

//-----------------------------------------------------------------------------
/*CreateTesterObjects creates the pointers necessary to communicate with the 
  tester.
 */
int TesterObjectsStruct::createTesterObjects(char *testerName1, int head1)
{
  //Try to connect to tester up to 7 times, waiting 1 sec in between tries.
  int NotConnected = 1;
  int SleepCnt = 0;

  while((NotConnected == 1) && (SleepCnt < 10)) {
    Tester = new TesterConnection(testerName1);
    if(Tester->getStatus() != EVXA::OK) {
      fprintf(stderr, "ERROR TesterConnection constructor: %s\n", Tester->getStatusBuffer());
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }
    cxnPtr = new TestheadConnection(testerName1, head1);
    if(cxnPtr->getStatus() !=  EVXA::OK) {
      fprintf(stderr, "ERROR in TestheadConnection constructor:%s\n%s\n", cxnPtr->getStatusBuffer(), testerName1);
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }
    Program = new ProgramControl(*cxnPtr);
    if(Program->getStatus() !=  EVXA::OK) {
      fprintf(stderr, "ERROR in Program constructor:%s\n", Program->getStatusBuffer());
      deleteTesterObjects(); 
      sleep(1);
      SleepCnt++;
      continue;
    }

    statePtr = new BasicStateNotification(*cxnPtr);
    if(statePtr->getStatus() !=  EVXA::OK) {
      fprintf(stderr, "ERROR in statePtr constructor:%s\n", Program->getStatusBuffer());
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }

    evxioPtr = new BasicEvxioStreamClient(testerName1, head1);
    if(evxioPtr->getStatus() != EVXA::OK) {
      fprintf(stderr, "ERROR in EvxioStreamClient constructor: %s", evxioPtr->getStatusBuffer());
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }
    int pid = getpid();
    char name[50];
    sprintf(name, "client_%d", pid);
    if(evxioPtr->ConnectEvxioStreams(cxnPtr, name) != EVXA::OK) {
      fprintf(stderr, "ERROR Connecting to evxio: %s\n", evxioPtr->getStatusBuffer());
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }
    if(evxioPtr->ConnectEvxioError() != EVXA::OK) {
      fprintf(stderr, "ERROR Connecting to evxio Error: %s\n", evxioPtr->getStatusBuffer());
      deleteTesterObjects();
      sleep(1);
      SleepCnt++;
      continue;
    }
    else
      NotConnected = 0;
    
    sleep(1);
    SleepCnt ++;
  }
  if(SleepCnt == 10)
    return -1;
  return 1;
  
}



//-----------------------------------------------------------------------------
/* DeleteTesterObjects is called when a connection is broken. 
*/
void TesterObjectsStruct::deleteTesterObjects()
{
  if(Tester != NULL) {
    delete Tester;
    Tester = NULL;
  }
  if(cxnPtr != NULL) {
    delete cxnPtr;
    cxnPtr = NULL;
  }
  if(Program != NULL) {
    delete Program;
    Program = NULL;
  }
  if(statePtr != NULL) {
    delete statePtr;
    statePtr = NULL;
  }
  if(evxioPtr != NULL) {
    delete evxioPtr;
    evxioPtr = NULL;
  }
}



//------------------------------------------------------------
void serverLoop();
void getTesterState();


//constants
const int NAMESIZE = 1024;


int selectLoop = 1;  // Flag to get back to select




//-----------------------------------------------------------------------------
/* Here's the main program.
 */
int main(int argc, char **argv)
{
 setDefaults();
 parseArgs(argc, argv);
  
  
 // Create a connection to a tester.
    TesterConnection tester(testerName);
    if (tester.getStatus() != EVXA::OK)
        {
        fprintf(stdout, "\n");
        fprintf(stderr, "Error in TesterConnection constructor:\n%s\n",
                tester.getStatusBuffer());
        exit(1);
        }
  
  int create_ok = TesterObjects.createTesterObjects(testerName, head);
  if(create_ok == -1) {// could not create objects so exit
    fprintf(stderr, "Could not connect to Tester %s, exiting\n", testerName);
    exit(1);
  }
  getTesterState();
  serverLoop();

  return(0);
}


//-----------------------------------------------------------------------------
/* Takes care of command line arguments.
 */
void parseArgs(int argc, char **argv)
{
  // If there are any arguments, parse them.  
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-tester")) {
      if ( ++i >= argc ) {
        fprintf(stderr, "ERROR: found -t/-tester argument but no tester name, tester '%s' will be used.\n", testerName);
      }
      else {
        strcpy(testerName, argv[i]);
      }
    }
    else if (!(strcmp(argv[i], "-head")) || !(strcmp(argv[i], "-hd"))) {
      if ( ++i >= argc ) {
        fprintf(stderr, "ERROR: found -head argument but no head number, head %d will be used.\n", head);
      } 
      else {
        head = atoi(argv[i]);
        if (!((head == 1) || (head == 2))) {
          head = 1;
          fprintf(stderr, "head '%s' not recognized, head %d will be used.\n", argv[i], head);
        }
      }
    }
    else if(!(strcmp(argv[i], "help")) || !(strcmp(argv[i], "-help"))) {
      fprintf(stdout, "usage %s -tester <tester_name> [-head <head_num>][-help]\n", argv[0]);
      executeHelp();
      exit(0);
    }
    else {
      fprintf(stderr, "ERROR: found unknown argument '%s'.\n", argv[i]);
      fprintf(stderr, "       Ignoring.\n");
    }
  }

  return;
}


//--------------------------------------------------------------------
/* Here's the function that checks the state of the tester when 
this application starts up.
*/
void getTesterState()
{
  //if not ready then TesterReady notification will tell us when 
  // the tester is ready.
  if(TesterObjects.Tester->isTesterReady(head)) 
    fprintf(stdout, "Tester ready for use\n");
  if(TesterObjects.Program->isProgramLoaded())
    fprintf(stdout, "Program is loaded\n");

}
  
  


//---------------------------------------------------------------------
/* Here's the main serverloop.  The select only cares about a few
inputs.  It cares about user input (stdin), state notifications, 
evxio messages, and evxio error stream.  Put these file descriptors in 
the select and wait for something to happen.
Return 0 to break from serverLoop.
*/
int main_serverLoop()
{
  fd_set read_fd;
  int stateSockId = TesterObjects.statePtr->getSocketId();
  int evxioSockId = TesterObjects.evxioPtr->getEvxioSocketId();
  int errorSockId = TesterObjects.evxioPtr->getErrorSocketId();

  FD_ZERO(&read_fd);
  FD_SET(fileno(stdin), &read_fd); //add input to select
  FD_SET(stateSockId, &read_fd); //add state notifications to select
  FD_SET(evxioSockId, &read_fd); // add evxio notifications to select
  FD_SET(errorSockId, &read_fd); // add error notifications to select
   
  int num_fds = ((stateSockId > evxioSockId) ? stateSockId : evxioSockId);
  num_fds = ((num_fds > errorSockId) ? num_fds + 1 : errorSockId +1);

  int num_ready;
  if((num_ready = select(num_fds, &read_fd, NULL, NULL, NULL)) < 0) {
    perror("main_serverLoop select failed ");
  }

  if(FD_ISSET(fileno(stdin), &read_fd)) // handle requests from stdin
    handleStdIn();

  if((stateSockId > 0) && (FD_ISSET(stateSockId, &read_fd))) {//handle requests for state notifications.
    if(TesterObjects.statePtr->respond(stateSockId) != EVXA::OK) {
      const char *errbuf = TesterObjects.statePtr->getStatusBuffer();
      fprintf(stdout, "ERROR state respond: %s\n", errbuf);
      testerReconnect = 1;
      return 0;
    }  
  }
  if((evxioSockId > 0) && (FD_ISSET(evxioSockId, &read_fd))) {//handle requests for evxio notifications.
    if(TesterObjects.evxioPtr->streamsRespond() != EVXA::OK) {
  //    const char *errbuf = TesterObjects.evxioPtr->getStatusBuffer();
  //    fprintf(stdout, "ERROR stream respond: %s\n", errbuf);
  //     testerReconnect = 1;
      return 0;
    }
  }
  if((errorSockId > 0) && (FD_ISSET(errorSockId, &read_fd))) {//handle requests for evxio notifications.
    if(TesterObjects.evxioPtr->ErrorRespond() != EVXA::OK) {
 //     const char *errbuf = TesterObjects.evxioPtr->getStatusBuffer();
 //     fprintf(stdout, "ERROR error respond: %s\n", errbuf);
 //     testerReconnect = 1;
      return 0;
    }
  }

  commandHandler();
  return 1;
}
    


//---------------------------------------------------------------------
/* This routine is used to setup the server loop.  In it is the controls 
to exit out of the loop and the program.
*/
void serverLoop()
{
  int mainLoop = 1;
  //fprintf(stderr, "Entering Server Loop\n");
  while(mainLoop == 1) { // This is to allow reconnects if needed
    selectLoop = 1;
    while((selectLoop == 1) && (exitServerLoop == 0)) { // This is to go back to select loop after select is handled.
      selectLoop = main_serverLoop();
    }
    if(testerReconnect == 1) {//try to reconnect to tester
      testerReconnect = 0;
      exitServerLoop = 0;
      fprintf(stderr, "serverLoop trying to reconnect\n");
      TesterObjects.deleteTesterObjects();
      int conn_ok = TesterObjects.createTesterObjects(testerName, head);
      if(conn_ok == 1)
        fprintf(stdout, "reconnect successful\n");
      else {
        fprintf(stdout, "reconnect unsuccessful, exiting\n");
        mainLoop = 0;
      }  
    }
    else // quit server and exit program.
      mainLoop = 0;
  }
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

    head = 1;
    strcpy(programName, "");

    return;
    }


