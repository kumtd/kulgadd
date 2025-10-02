////////////////////////////////////////////////////////////////////////////////
///
///   global.hh
///
///   Declarations of globle variables
///
///   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
///            Kyungmin Lee (  railroad@korea.ac.kr)
///            Changi Jeong (  jchg3876@korea.ac.kr)
///
////////////////////////////////////////////////////////////////////////////////



#pragma once


#include "SerialManager.hh"
#include "WebSocketServer.hh"
#include "PinGrid.hh"
#include "ScanManager.hh"



extern unsigned short int gVerbose;

extern SerialManager* gSerial;
extern WebSocketServer* gServer;
extern PinGrid* gGrid;
extern ScanManager* gScan;
