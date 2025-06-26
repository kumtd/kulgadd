////////////////////////////////////////////////////////////////////////////////
//
//   main.cc
//
//   Main function of KU MTD daemon.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//            Changi Jeong (  jchg3876@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <iostream>
#include <getopt.h>
#include <string.h>

#include "SerialManager.hh"



//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------
enum ExitCode
{
	SUCCESS           =   0,
	TERM_SIGNAL       =   1,
	ERROR_SERIAL_CONN =   2,
	ERROR_UNKNOWN     = 100
};



//------------------------------------------------------------------------------
// Declaration of print_help function.
//------------------------------------------------------------------------------
void print_help();



//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//------------------------------------------------
	// Read options
	//------------------------------------------------
	// Options flags
	unsigned short int flag_v = 0; // verbose
	unsigned short int flag_s = 0; // switch device

	// Option containers
	unsigned short int verbose = 0;
	char* dev_switch = NULL;

	// Option dictionary
	const char* const short_options = "hv:s:";
	const struct option long_options[] = {
		{"help"    , 0, NULL, 'h'},
		{"verbose" , 1, NULL, 'v'},
		{"switch"  , 1, NULL, 's'},
		{NULL      , 0, NULL,   0}
	};

	// Read options
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
				verbose = atoi(optarg);
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


	//------------------------------------------------
	// Option inspection
	//------------------------------------------------



	//------------------------------------------------
	// Open serial
	//------------------------------------------------
	// Define serial manager
	SerialManager serial;

	// Open serial connection
	if ( flag_s )
	{
		if ( !serial . Init(dev_switch) )
		{
			std::cerr << "[kumtdd] Failed to open serial port " << dev_switch << std::endl;
			return ERROR_SERIAL_CONN;
		}
		else
		{
			std::cout << "[kumtdd] Open serial port " << dev_switch << std::endl;
		}
		
	}
	else
	{
		// Currently the serial device is hard-coded,
		// but later will be searched automatically by finding raspberry pi pico
		if ( !serial . Init("/dev/ttyACM0") )
		{
			std::cerr << "[kumtdd] Failed to open serial port /det/ttyACM0" << std::endl;
			return ERROR_SERIAL_CONN;
		}
		else
		{
			std::cout << "[kumtdd] Open serial port /det/ttyACM0" << std::endl;
		}
	}


	//--------------------------------------------------
	// Finalize
	//--------------------------------------------------
	return SUCCESS;
}



//------------------------------------------------------------------------------
// Definition of print_help function.
//------------------------------------------------------------------------------
void print_help()
{
	std::cout << "Usage: kumtdd [OPTION]..." << std::endl;
	std::cout << "KU MTD daemon." << std::endl;
	std::cout << std::endl;
	std::cout << "Examples:" << std::endl;
	std::cout << "  kumtdd --verbose 1  # Execute the daemon with verbose level 1" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -v, --verbose  Set verbose level"                                                << std::endl;
	std::cout << "  -s, --switch   Manually designate switching matrix controller, that is, pi pico" << std::endl;
}
