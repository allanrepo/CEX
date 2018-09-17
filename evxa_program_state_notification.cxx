//////////////////////////////////////////////////////////////////////
//             Copyright 1999, LTX Corporation, Westwood, MA
//
// 
// evxa_basic_state_notification.cxx  --  This program will demonstrate the basic
//                     features of the EVXA library, performing
//                     useful operations using the different classes
//                     that are provided in the library.
//
//
//
//  This file has all the Notification functions defined.  Each Notification
//  function calls a handle function hwere the user action can be coded.
//  Code the user function in the handle routines located in evxa_basic_entry_points.cxx.
//
//  Revision History:
//
//  99-05-11  Jim Panopulos  Created
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <evxa_program_demo.hxx>



//---- External Functions -----------------------------------------------------
extern void handleLoadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleEndLoadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleSaveTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleEndSaveTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleUnloadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleEndUnloadTestProg(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleStartTest(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleReset(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleEndOfReset(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleChangeState(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleUserChangeState(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleChangeDlog(const EVX_DLOG_STATE state, const char *text_msg);
extern void handleEndOfTest(const int array_size,int site[],int serial[],int swbin[],int hwbin[],int pass[]);
extern void handleTcBooting();
extern void handleTesterReady();
extern void handleEvxioMessage(int responseNeeded, int responseAquired, const char *evxio_msg);
extern void handleLoadCont(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleOnDemandCont(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleOnLoadCont(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleSaveCont(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleOverlayCont(const EVX_PROGRAM_STATE state, const char *text_msg);
extern void handleErrorMessage(evx_err_msgrec errMsg);


/*************************************************************************************** 
***************************************************************************************/ 
void BasicStateNotification::programChange(const EVX_PROGRAM_STATE state, const char *text_msg) 
{ 
  switch (state) { 

  case EVX_PROGRAM_LOADING: 
    { 
      handleLoadTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_LOADED: 
    { 
      handleEndLoadTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_SAVING: 
    { 
      handleSaveTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_SAVED: 
    { 
      handleEndSaveTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_UNLOADING: 
    { 
      handleUnloadTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_UNLOADED: 
    { 
      handleEndUnloadTestProg(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_START: 
    { 
      handleStartTest(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_RESET: 
    { 
      handleReset(state, text_msg);
    } 
  break; 
  
  case EVX_PROGRAM_RESET_DONE: 
    { 
      handleEndOfReset(state, text_msg);
    } 
  break; 

  case EVX_PROGRAM_PATLOADING: 
  case EVX_PROGRAM_CALFILE_LOAD: 
  case EVX_PROGRAM_RESOLVING:
  case EVX_PROGRAM_READY:
  case EVX_PROGRAM_RUNNING:
  case EVX_PROGRAM_CAD:
  case EVX_PROGRAM_UTL:
  case EVX_PROGRAM_NONE:
    { 
      handleChangeState(state, text_msg);
    } 
  break; 

  case EVX_USER_DEFINED:
    {
      handleUserChangeState(state, text_msg);
    } 
  break; 

  case EVX_ENVISION_DLOG_CHANGE:
    {
      handleChangeDlog(MAX_EVX_DLOG_STATE, text_msg); 
    } 
  break; 
  
  case EVX_PROGRAM_LOAD_CONT:
    {
      handleLoadCont(state, text_msg);
    }
  break;

  case EVX_PROGRAM_ON_DEMAND_CONT:
    {
      handleOnDemandCont(state, text_msg);
    }
  break;

  case EVX_PROGRAM_ON_LOAD_CONT:
    {
      handleOnLoadCont(state, text_msg);
    }
  break;

  case EVX_PROGRAM_SAVE_CONT:
    {
      handleSaveCont(state, text_msg);
    }
  break;

 case EVX_PROGRAM_OVERLAY_CONT:
    {
      handleOverlayCont(state, text_msg);
    }
  break;

  default: 
    //fprintf (stderr, "BasicStateNotification programChange unknown comand: %d %s\n", state, EV_NS(text_msg));
    break; 
  }  // end switch 
} 
  

/*************************************************************************************** 
***************************************************************************************/ 
void BasicStateNotification::programRunDone(const int array_size,int site[],
                                            int serial[],int swbin[],
                                            int hwbin[],int pass[],
                                            unsigned int)
{ 
  handleEndOfTest(array_size, site, serial, swbin, hwbin, pass);
 
  //delete the arguments
  if(site != NULL) {
    delete []site;
    site = NULL;
  }
  if(serial != NULL) {
    delete []serial;
    serial = NULL;
  }
  if(hwbin != NULL) {
    delete []hwbin;
    hwbin = NULL;
  }
  if(swbin != NULL) {
    delete []swbin;
    swbin = NULL;
  }
  if(pass != NULL) {
    delete []pass;
    pass = NULL;
  }
} 
  


/*************************************************************************************** 
***************************************************************************************/ 
void BasicStateNotification::dlogChange(const EVX_DLOG_STATE state)
{
  handleChangeDlog(state, "dlogChange Notification");
}


/*************************************************************************************
*************************************************************************************/
void BasicStateNotification::tcBooting(void)
{
  handleTcBooting();
}


/*************************************************************************************
*************************************************************************************/
void BasicStateNotification::testerReady(void)
{
  handleTesterReady();
}


/**************************************************************************************
***************************************************************************************/
void BasicEvxioStreamClient::EvxioMessage(int responseNeeded, int responseAquired, const char *evxio_msg)
{
  handleEvxioMessage(responseNeeded, responseAquired, evxio_msg);
}


/**************************************************************************************
***************************************************************************************/
 void BasicEvxioStreamClient::ErrorMessage(evx_err_msgrec errMsg)
{
    handleErrorMessage(errMsg);
}

/**************************************************************************************
***************************************************************************************/
 void BasicEvxioStreamClient::ErrorResponseMessage(evx_err_recv_rec reply)
{
    fprintf (stderr, "ErrorResponseMessage received reply\n");
}
