//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_basic_entry_points.cxx  --  This program will demonstrate the basic
//                     features of the EVXA library, performing
//                     useful operations using the different classes
//                     that are provided in the library.
//
//
//
//  This file contains the user handle functions that are called by notifications.
//  This is the file that the user will add to and be custom.  All the handle 
//  routines are called by the functions in the evxa_basic_state_notification.cxx
//  file.  I moved them to a seperate file so this file can be modified by the user
//  and the notification functions can stay intact.
//
//  Revision History:
//
//  99-05-11  Jim Panopulos  Created
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <evxa_program_demo.hxx>


//---- External Globals -----------------------------------------------------
extern struct TesterObjectsStruct TesterObjects;
extern int resultsOn;
extern int startTestOn;
extern int programReady;
extern int evxioResponseFlag;

//----------------------------------------------------------------------------
// handleLoadTestProg gets called when the test program has started to load.
void handleLoadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "Loading %s...\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleEndLoadTestProg is called when the test program has completed loading
// and the OnLoadFlow has executed.
void handleEndLoadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "End Load Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleSaveTestProg is called when the test program has started to be saved.
void handleSaveTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "Saving Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleEndSaveTestProg is called when the test program has completed being 
// saved.
void handleEndSaveTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  TesterObjects.Program->clearStatus();
  const char *prog = TesterObjects.Program->getProgramPath();
  if (strcmp(prog, text_msg) != 0) {
      TesterObjects.Program->setProgramPath(text_msg);
      prog = TesterObjects.Program->getProgramPath();
  }
 
  fprintf(stdout, "End Save Test Prog: %s\nState: %d\n", prog, state);
}


//----------------------------------------------------------------------------
// handleUnloadTestProg is called when the test program has started to be 
// unloaded.
void handleUnloadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "Unloading Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleEndUnloadTestProg is called when the test program hs completed unloading.
void handleEndUnloadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "End Unload Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleStartTest is called when the test program has started execution.
void handleStartTest(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if((resultsOn == 1) && (startTestOn == 1))
    fprintf(stdout, "Start Test: %s\nState: %d\n", text_msg, state);
  programReady = 0;
}


//----------------------------------------------------------------------------
// handleReset is called when the test program starts reset.
void handleReset(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "Reset Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleEndOfReset is called when the test program completes reset.
void handleEndOfReset(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "End Reset Test Prog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleChangeState is called for specific state changes such as pattern resolving.
// see evxa_core_state_notification.cxx for specif states that call this function.
void handleChangeState(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "Change State: %s\nState: %d\n", text_msg, state);
  if ((state == EVX_PROGRAM_READY) && (!startTestOn)) programReady = 1;
}


//----------------------------------------------------------------------------
// handleUserChangeState is called when a user defined state occured.
void handleUserChangeState(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  fprintf(stdout, "User State: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleChangeDlog is called when the dlog state of a program is changed.
void handleChangeDlog(const EVX_DLOG_STATE state, const char *text_msg)
{
  fprintf(stdout, "Change Dlog: %s\nState: %d\n", text_msg, state);
}


//----------------------------------------------------------------------------
// handleEndOfTest is called when the test program completes execution.
// The bin results are available from this entry point.
void handleEndOfTest(const int array_size,int site[],int serial[],int swbin[],int hwbin[],int pass[])
{
  serial++; serial--; //fix unused compiler warning temporarily
  if(startTestOn == 1) {
    startTestOn = 0;  //turn off the start test flag.
    if(resultsOn == 1) {
      for(int ii=1; ii<array_size; ii++) {
        if(site[ii] == 1) { //valid site
          fprintf(stdout, "SITE %d HW_BIN=%d SW_BIN=%d %s\n", ii, hwbin[ii], swbin[ii],
	      pass[ii] == 1 ? "PASS" : "FAIL");
        }
      }
    }
  }
}



//----------------------------------------------------------------------------
// handleEvxioMessage is called when there is data on an IO Stream this application
// is interested in.  
void handleEvxioMessage(int responseNeeded, int responseAquired, const char *evxio_msg)
{
  if(evxio_msg != NULL)
    fprintf(stdout, "%s", evxio_msg);
  
  if(responseNeeded == 1)
    evxioResponseFlag = 1;
  
  if(responseAquired == 1)
    evxioResponseFlag = 0;
}



//----------------------------------------------------------------------------
// handleErrorMessage is called when there is data on an IO Stream this application
// is interested in.  
void handleErrorMessage(evx_err_msgrec errMsg)
{
    EVX_ERR_SEVERITY severity;
    std::vector<std::string> buttonList;
    std::string severityText;

    TesterObjects.evxioPtr->ErrorButtons(&errMsg, severity, buttonList, severityText);
    int num_buttons = buttonList.size();
    fprintf(stdout, "ERROR (%d) %s NumButtons=%d\n", (int)severity, severityText.c_str(), num_buttons);
    int len = strlen(errMsg.error_msg);
    fprintf(stdout, "%d: %s\n", len, errMsg.error_msg);
    for (int ii=0; ii<num_buttons; ii++) {
	fprintf(stdout, "\t%s\n", buttonList[ii].c_str());
    }

#if 0
    ErrorResponseType err_response = err_NoneButton;
    switch (severity) {
    case EVX_ERR_UNKNOWN: 
    case EVX_ERR_ADVISORY: 	// no dialog
    case EVX_ERR_WARNING:	// no dialog
    case EVX_ERR_CRITICAL:	// no dialog
	break;
    case EVX_ERR_FATAL:		// reset dialog
	err_response = err_ResetButton;
	break;
    case EVX_ERR_DIALOG_YES_NO:	// yes/no dialog
	err_response = err_YesButton;
	break;
    case EVX_ERR_DIALOG_OK:	// ok dialog
	err_response = err_AckButton;
	break;
    }
     if (err_response != err_NoneButton)
    	TesterObjects.evxioPtr->ErrorResponse(err_response);
#endif
}

//----------------------------------------------------------------------------
// handleTcBooting is called when the tester has started booting
//   
void handleTcBooting()
{
  fprintf(stdout, "TC Booting\n"); 
}



//----------------------------------------------------------------------------
// handleTesterReady is called when the tester is ready for use
//   
void handleTesterReady()
{
  fprintf(stdout, "Tester ready for use\n");
}


//-----------------------------------------------------------------------------
// handleLoadCont is called when load continue notifies come
//
void handleLoadCont(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if(text_msg != NULL)
    fprintf(stderr, "LOAD_CONT: %s\nState: %d\n", text_msg, state);
  else
    fprintf(stderr, "LOAD_CONT: text_msg is NULL\n");
}


//-----------------------------------------------------------------------------
// handleOnDemandCont is called when on demand conte notifies come
//
void handleOnDemandCont(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if(text_msg != NULL)
    fprintf(stderr, "ON_DEMAND_CONT: %s\nState: %d\n", text_msg, state);
  else
    fprintf(stderr, "ON_DEMAND_CONT: text_msg is NULL\n");
}


//-----------------------------------------------------------------------------
// handleOnLoadCont is called when on load continue notifies come
//
void handleOnLoadCont(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if(text_msg != NULL)
    fprintf(stderr, "ON_LOAD_CONT: %s\nState: %d\n", text_msg, state);
  else
    fprintf(stderr, "ON_LOAD_CONT: text_msg is NULL\n");
}


//-----------------------------------------------------------------------------
// handleSaveCont is called when save continue notifies come
//
void handleSaveCont(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if(text_msg != NULL)
    fprintf(stderr, "SAVE_CONT: %s\nState: %d\n", text_msg, state);
  else
    fprintf(stderr, "SAVE_CONT: text_msg is NULL\n");
}
 

//-----------------------------------------------------------------------------
// handleOverlayCont is called when overlay continue notifies come
//
void handleOverlayCont(const EVX_PROGRAM_STATE state, const char *text_msg)
{
  if(text_msg != NULL)
    fprintf(stderr, "OVERLAY_CONT: %s\nState: %d\n", text_msg, state);
  else
    fprintf(stderr, "OVERLAY_CONT: text_msg is NULL\n");
}

