////////////////////////////////////////////////////////////////////////////////
///
///   main.cc
///
///   Main function of KU LGAD daemon.
///
///   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
///            Kyungmin Lee (  railroad@korea.ac.kr)
///            Changi Jeong (  jchg3876@korea.ac.kr)
///
////////////////////////////////////////////////////////////////////////////////



///-----------------------------------------------------------------------------
/// Headers
///-----------------------------------------------------------------------------
#include <iostream>
#include <getopt.h>
#include <string.h>
#include <csignal>

#include "global.hh"
#include "SerialManager.hh"
#include "PinGrid.hh"
#include "WebSocketServer.hh"
#include "ScanManager.hh"



///-----------------------------------------------------------------------------
/// Global variables
///-----------------------------------------------------------------------------
enum exit_code
{
	SUCCESS           =   0,
	TERM_SIGNAL       =   1,
	ERROR_SERIAL_CONN =   2,
	ERROR_UNKNOWN     = 100
};
unsigned short int gVerbose = 0;

SerialManager*   gSerial = 0;
WebSocketServer* gServer = 0;
PinGrid*         gGrid   = 0;
ScanManager*     gScan   = 0;



///-----------------------------------------------------------------------------
/// Declaration of print_help function.
///-----------------------------------------------------------------------------
void print_help();



///-----------------------------------------------------------------------------
/// Signal handler
///-----------------------------------------------------------------------------
std::atomic<bool> terminateRequested = false;
void signalHandler(int sig)
{
	terminateRequested = true;
}



///-----------------------------------------------------------------------------
/// main
///-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//----------------------------------------------------------
	// Signal handler 
	//----------------------------------------------------------
	std::signal(SIGINT, signalHandler);


	//----------------------------------------------------------
	// Read options
	//----------------------------------------------------------
	//--------------------------------------
	// Options flags
	//--------------------------------------
	unsigned short int flag_v = 0; // verbose
	unsigned short int flag_s = 0; // switch device

	//--------------------------------------
	// Option containers
	//--------------------------------------
	char* dev_switch = "/dev/ttyACM0";

	//--------------------------------------
	// Option dictionary
	//--------------------------------------
	const char* const short_options = "hv:s:";
	const struct option long_options[] = {
		{"help"    , 0, NULL, 'h'},
		{"verbose" , 1, NULL, 'v'},
		{"switch"  , 1, NULL, 's'},
		{NULL      , 0, NULL,   0}
	};

	//--------------------------------------
	// Read options
	//--------------------------------------
	int opt;
	do
	{
		opt = getopt_long(argc, argv, short_options, long_options, NULL);

		switch ( opt )
		{
			case 'h':
				print_help();
				break;

			case 'v':
				flag_v = 1;
				gVerbose = atoi(optarg);
				break;

			case 's':
				flag_s = 1;
				dev_switch = strdup(optarg);
				break;

			case '?':
				print_help();
				break;

			case -1:
				break;

			default:
				abort();
		}
	} while ( opt != -1 );


	//----------------------------------------------------------
	// Option inspection
	//----------------------------------------------------------
	// Need to check whether switch device exists and it's pi pico. Let's make it later.
	// Also, currently the default serial device is hard-coded as /dev/ttyACM0,
	// but later will be searched automatically by finding raspberry pi pico


	//----------------------------------------------------------
	// Open serial
	//----------------------------------------------------------
	//--------------------------------------
	// Define serial manager
	//--------------------------------------
	gSerial = new SerialManager();

	//--------------------------------------
	// Open serial connection
	//--------------------------------------
	if ( !gSerial -> Connect() )
	{
		std::cerr << "[kumtdd::main] Failed to open serial port " << dev_switch << std::endl;
		return ERROR_SERIAL_CONN;
	}
	else
	{
		std::cout << "[kumtdd::main] Open serial port " << dev_switch << std::endl;
	}


	//----------------------------------------------------------
	// Define pin grid
	//----------------------------------------------------------
	gGrid = new PinGrid();


	//----------------------------------------------------------
	// Define scan manager
	//----------------------------------------------------------
	gScan = new ScanManager();
	// The script hardcoded, but will be modified later
	gScan -> SetCMD("python3 /sw/kulgadd/dev/source/scripts/iv_all.py --Vstart 0 --Vend -50 --Vstep 1 --sensorname w5a --Icompliance 1e-5");


	//----------------------------------------------------------
	// Websocket 
	//----------------------------------------------------------
	try
	{
		gServer = new WebSocketServer(gSerial, gGrid);
		gServer -> Start();
		while ( ! terminateRequested )
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "[kumtdd::main] Fatal error: " << e . what() << std::endl;
		return 1;
	}


	//----------------------------------------------------------
	// Finalize
	//----------------------------------------------------------
	gServer -> Stop();
	delete gScan;
	delete gServer;
	delete gGrid;
	delete gSerial;
	return SUCCESS;
}



///-----------------------------------------------------------------------------
/// Definition of print_help function.
///-----------------------------------------------------------------------------
void print_help()
{
	std::cout << "Usage: kumtdd [OPTION]..." << std::endl;
	std::cout << "KU MTD daemon." << std::endl;
	std::cout << std::endl;
	std::cout << "Examples:" << std::endl;
	std::cout << "  kumtdd --verbose 1  # Execute the daemon with verbose level 1" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -v, --verbose  Set verbose level"                              << std::endl;
	std::cout << "  -s, --switch   Manually designate switching matrix controller" << std::endl;
}
