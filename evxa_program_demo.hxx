//
// Copyright (C) Trillium A Division of LTX 1999
//               ALL RIGHTS RESERVED
//
// Author: Jim Panopulos
// Module: evxa_core_program.hxx
// Created: May 13, 1999
// Description:
//

#ifndef EVXA_BASIC_PROGRAM_HXX
#define EVXA_BASIC_PROGRAM_HXX

#include <evxa/ProgramControl.hxx>
#include <evxa/StateNotification.hxx>
#include <evxa/EvxioStreamsClient.hxx>


class BasicStateNotification;  //forward declaration of class
class BasicEvxioStreamClient;  //forward declaration of class
class TesterObjectsStruct;     //forward declaration of class


//---- Tester objects structure ------------------------------
class TesterObjectsStruct 
{
public:
  TesterConnection *Tester;
  TestheadConnection *cxnPtr;
  ProgramControl *Program;
  BasicStateNotification *statePtr;
  BasicEvxioStreamClient *evxioPtr;
  TesterObjectsStruct();
  ~TesterObjectsStruct();
  int createTesterObjects(char *testerName, int head);
  void deleteTesterObjects();
};


//---- State Notification Class --------------------------------
class BasicStateNotification : public StateNotification
{
public:
  BasicStateNotification(const TestheadConnection &thc):StateNotification(thc) {}
  void programChange(const EVX_PROGRAM_STATE state, const char *text_msg);
  void programRunDone(const int array_size,int site[],int serial[],int swbin[],int hwbin[],int pass[],unsigned int dsp_status);
  void dlogChange(const EVX_DLOG_STATE state);
  void tcBooting(void);
  void testerReady(void);
 
};


//---- evxio streams client class ------------------------------
class BasicEvxioStreamClient : public EvxioStreamsClient
{
public:
    BasicEvxioStreamClient(char *tester_name, int headNum):EvxioStreamsClient(tester_name, headNum) {}
    void EvxioMessage(int responseNeeded, int responseAquired, const char *evxio_msg);
    void ErrorMessage(evx_err_msgrec errMsg);
    void ErrorResponseMessage(evx_err_recv_rec reply);
};



// Local Variables:
// c-file-style: "Ellemtel"
// mode: c++
// End:
#endif

