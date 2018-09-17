//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_basic_input.cxx  --  This program will demonstrate the basic
//                     features of the EVXA library, performing
//                     useful operations using the different classes
//                     that are provided in the library.
//
//
//
//  This file is used to handle all the user input.  All functions that
//  are created to talk to the tester can be placed here.
//
//  Revision History:
//
//  99-10-19  Yufeng Xiong   Added test case for is_cadence_program_changed
//  99-05-11  Jim Panopulos  Created
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <evxa_program_demo.hxx>
#include <iostream>


//---- External Globals -----------------------------------------------------
extern struct TesterObjectsStruct TesterObjects;
extern int exitServerLoop;  //Flag to quit server loop
extern int testerReconnect; //Flag to reconnect to tester.


//---- Globals --------------------------------------------------------------
int startTestOn = 0;
int programReady = 1;
int resultsOn = 0;
int evxioResponseFlag = 0;
int ntimes = 0;
int startIsLooping = 0;
int quitAfterLoop = 0;



//-----------------------------------------------------------------
void commandHandler()
{
    // do a pass of the start loop
    if (programReady && (!startTestOn) && (ntimes > 0))
    {
        if ((!startIsLooping) && (ntimes > 1)) {
            startIsLooping = ntimes;}
        startTestOn = 1;
        ntimes--;

        TesterObjects.Program->clearStatus();
        if (TesterObjects.Program->start(EVXA::NO_WAIT) != EVXA::OK)
        {
            fprintf (stderr, "Execution count-down run number: %d had error\n",
                     ntimes);
            fprintf (stderr, "ERROR executeTest: %s\n",
                     TesterObjects.Program->getStatusBuffer());
            TesterObjects.Program->clearStatus();
        }
    }
    if ((startIsLooping) && (!ntimes) && (programReady) && (!startTestOn)) 
    {
        fprintf (stdout, "Start-loop of %d runs completed\n", 
                startIsLooping);
        startIsLooping = 0;
        if (quitAfterLoop) {
            exitServerLoop = 1;}
    }
}


//-----------------------------------------------------------------
void executeStart (char *data)
{
    // Make sure program is loaded
    if (TesterObjects.Program->isProgramLoaded())
    {
        ntimes = 1;        // default is one start
        quitAfterLoop = 0; // default is to not quit after start-loop

        // See if user specified a count of times to start 
        sscanf (data, "%*s %d %d", &ntimes, &quitAfterLoop);
        if (ntimes < 1)
        {
            fprintf (stderr, "ERROR executeStart: invalid count %d\n", ntimes);
        }
        else if (ntimes > 1)
        {
            printf ("Will execute %d times\n", ntimes);
        }
    }
    else
    {
        printf("Program not loaded\n");
    }
}




//---------------------------------------------------------------
void executeLoad(char *data)
{
  if(TesterObjects.Program->isProgramLoaded()) {
    fprintf(stdout, "Program already loaded\n");
    return;
  }

  int buflen = strlen(data);
  data[buflen-1] = '\0'; //get rid of RETURN char.
  char* l;
  char *parseString = strtok_r(data, " ", &l);  // get the command
  parseString = strtok_r(NULL, " ", &l);
  if (parseString != NULL) {
    TesterObjects.Program->clearStatus();
    if(TesterObjects.Program->load(parseString, EVXA::NO_WAIT) != EVXA::OK) {
      const char * errBuf = TesterObjects.Program->getStatusBuffer();
      fprintf(stderr, "ERROR Program Load: %s\n", errBuf);
      TesterObjects.Program->clearStatus();
    } else {
      fprintf(stderr, "Loading program %s, please wait ..\n", parseString);
    }
  }
  else
    fprintf(stderr, "ERROR executeLoad: load <file> (no file found)\n");
}

//-----------------------------------------------------------------
void executeUnLoad()
{
  if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to unload
    TesterObjects.Program->clearStatus();
    if(TesterObjects.Program->unload(EVXA::NO_WAIT) != EVXA::OK) {
      const char *errBuf = TesterObjects.Program->getStatusBuffer();
      fprintf(stdout, "ERROR Program Unload: %s\n", errBuf);
      TesterObjects.Program->clearStatus();
    }
  }
  else
    fprintf(stdout, "Program not loaded\n");
}



//-----------------------------------------------------------------
void executeReset()
{
  if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to reset
    TesterObjects.Program->clearStatus();
    if(TesterObjects.Program->reset() != EVXA::OK) {
      const char *errBuf = TesterObjects.Program->getStatusBuffer();
      fprintf(stdout, "ERROR Program Reset: %s\n", errBuf);
      TesterObjects.Program->clearStatus();
    }
  }
  else
    fprintf(stdout, "Program not loaded\n");
}



//-----------------------------------------------------------------
void executeHelp()
{
  fprintf(stdout, "Here is a list of valid commands\n");
  fprintf(stdout, "quit - exits the program\n");
  fprintf(stdout, "reconnect - reconnect to the tester\n");
  fprintf(stdout, "start [ n ] - execute the test, optionally n times\n");
  fprintf(stdout, "results - display test results\n");
  fprintf(stdout, "noresults - turn off test results\n");
  fprintf(stdout, "load <filename> - load the given file\n");
  fprintf(stdout, "reset - issue a reset to the tester\n");
  fprintf(stdout, "unload - unload the loaded program\n");
  fprintf(stdout, "gettestersites - get the tester selected sites\n");
  fprintf(stdout, "getwafer - get the die coordinates for the selected sites\n");
  fprintf(stdout, "getrobots - list available robot names\n");
  fprintf(stdout, "robotstart - start the active robot\n");
  fprintf(stdout, "robotstop - stop the active robot\n");
  fprintf(stdout, "activerobot - get the name of the active robot\n");
  fprintf(stdout, "setactiverobot - set the active robot\n");
  fprintf(stdout, "help - display command help\n");
}

//-----------------------------------------------------------------
void executeEvxioResponse(char *buf)
{
  if(TesterObjects.evxioPtr->EvxioResponse(buf) != EVXA::OK) {
    const char *errBuf = TesterObjects.evxioPtr->getStatusBuffer();
    fprintf(stderr, "ERROR evxio response: %s\n", errBuf);
    TesterObjects.evxioPtr->clearStatus();
  }
}
      

//-----------------------------------------------------------------
void executeTestersites()
{
    const int num_sites = 5;
    int sites[num_sites] = {1, 1, 0, 1, 0};
    
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	TesterObjects.Program->clearStatus();
	TesterObjects.Program->setSelectedSites(num_sites, sites);
    }
}

//-----------------------------------------------------------------
void executeAddsites()
{
    const int num_sites = 5;
    int sites[num_sites] = {1, 1, 1, 1, 1};
    
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	TesterObjects.Program->clearStatus();
	TesterObjects.Program->setSelectedSites(num_sites, sites);
    }
}

//-----------------------------------------------------------------
void executeGetTestersites()
{
   
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	int num_sites = TesterObjects.Program->getNumberOfSelectedSites();
	std::cout<< "executeGetTestersites num_sites=" << num_sites << std::endl;
	if (num_sites > 0) {
	    int *sites = TesterObjects.Program->getSelectedSites();
	    for (int ii=1; ii<num_sites; ii++) {
		std::cout<< "site"<<ii<<" " << sites[ii] << std::endl;
	    }
	}
	
    }
}

//-----------------------------------------------------------------
void executeSetWafer()
{
    const int num_sites = 5;
    int sites[num_sites] = {1, 1, 1, 1, 1};
    int xcoord[num_sites] = {0, -1, -2, 3, 4};
    int ycoord[num_sites] = {0, 10, 11, 12, 13};
    
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	TesterObjects.Program->clearStatus();
	TesterObjects.Program->setWaferCoords(num_sites, sites, xcoord, ycoord);
    }
}

//-----------------------------------------------------------------
void executeGetWafer()
{
   
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	int num_sites = TesterObjects.Program->getNumberOfSelectedSites();
	std::cout<< "executeGetTestersites num_sites=" << num_sites << std::endl;
	if (num_sites > 0) {
	    int *sites = new int[num_sites];
	    int *xcoord = new int[num_sites];
	    int *ycoord = new int[num_sites];
	    TesterObjects.Program->getWaferCoords(num_sites, sites, xcoord, ycoord);
	    for (int ii=1; ii<num_sites; ii++) {
		fprintf(stdout, "site%d: %d %d %d\n", ii, sites[ii], xcoord[ii], ycoord[ii]);
	    }
	    delete []sites;
	    delete []xcoord;
	    delete []ycoord;
	}	
    }
}

//-----------------------------------------------------------------
void executeGetRobots()
{
   
    if(TesterObjects.Program->isProgramLoaded()) { //program is loaded so ok to programvalidate
	std::vector<std::string> robotList;
	TesterObjects.Program->getRobotNames(robotList);
	int num_robots = robotList.size();
	fprintf(stdout, "numrobots %d\n", num_robots);
	for (int ii=0; ii<num_robots; ii++)
	    fprintf(stdout,"%s\n", robotList[ii].c_str());
    }
}

void executeRobotStart()
{
    TesterObjects.Program->faprocInvoke("start");
}


void executeRobotStop()
{
    TesterObjects.Program->faprocInvoke("End Lot", "wait_end_DUT_test");
}


void executeActiveRobot()
{
    std::string obj_name = TesterObjects.Program->getActiveExtInterfaceObject();    
    std::string robot_name = TesterObjects.Program->getRobotEquipFromObject(obj_name); 
    fprintf(stdout, "Active ExtIntf: %s Robot: %s\n", obj_name.c_str(), robot_name.c_str());
}
	
void executeSetActiveRobot(char *data)
{
    int len = strlen(data); //whole command, including "setactiverobot"
    char *newname = NULL;
    if ( len > 0 ) {
        char* l;
        data[len-1] = '\0'; //null-terminate it
        newname = strtok_r(data, " ", &l);  //command name
	newname = strtok_r(NULL, " ", &l);
	fprintf(stdout, "SetActiveRobot: %s\n", newname);
	TesterObjects.Program->setActiveExtInterfaceObject(newname);
    }
}


//-----------------------------------------------------------------
// This is the callback that handles Notifications coming from stdin.
void handleStdIn()
{
  char buf[BUFSIZ] = "";

  read(fileno(stdin), buf, BUFSIZ);
  
  if(evxioResponseFlag == 1) { //send the string back to evxio
    evxioResponseFlag = 0;
    executeEvxioResponse(buf);
  }
  else if(!strncmp(buf,"quit",4)) { //exit program
    exitServerLoop = 1;
  }
  else if(!strncmp(buf, "reconnect", 9)) { // reconnect to tester
    exitServerLoop = 1;
    testerReconnect = 1;
  }
  else if (!strncmp(buf, "start", 5)) { //send the test command
    executeStart(buf);
  }
  else if (!strncmp(buf, "results", 7)) { //show test results if tester is running
    resultsOn = 1;
  }
  else if (!strncmp(buf, "noresults", 9)) { // turn off test results
    resultsOn = 0;
  }
  else if(!strncmp(buf, "load", 4)) {
    executeLoad(buf);
  }
  else if(!strncmp(buf, "unload", 6)) {
    executeUnLoad();
  }
  else if(!strncmp(buf, "reset", 5)) {
    executeReset();
  }
  else if(!strncmp(buf, "help", 4)) {
    executeHelp();
  }
  else if(!strncmp(buf, "testersites", 11)) {
    executeTestersites();
  }
  else if(!strncmp(buf, "addsites", 8)) {
    executeAddsites();
  }
  else if(!strncmp(buf, "gettestersites", 14)) {
    executeGetTestersites();
  }
  else if(!strncmp(buf, "setwafer", 8)) {
    executeSetWafer();
  }
  else if(!strncmp(buf, "getwafer", 8)) {
    executeGetWafer();
  }
  else if(!strncmp(buf, "getrobots", 9)) {
    executeGetRobots();
  }
  else if(!strncmp(buf, "robotstart", 10)) {
    executeRobotStart();
  }
  else if(!strncmp(buf, "robotstop", 9)) {
    executeRobotStop();
  }
  else if(!strncmp(buf, "activerobot", 11)) {
    executeActiveRobot();
  }
  else if(!strncmp(buf, "setactiverobot", 9)) {
    executeSetActiveRobot(buf);
  }
  else 
    fprintf(stdout, "Invalid command: %s\n", buf);
}
