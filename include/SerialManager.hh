////////////////////////////////////////////////////////////////////////////////
//
//   SerialManager.hh
//
//   This class opens serial port, sends commands, and handles response.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//            Changi Jeong (  jchg3876@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



#pragma once



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <cstddef>
#include "PinGrid.hh"



//------------------------------------------------------------------------------
// Class declaration
//------------------------------------------------------------------------------
class SerialManager
{
	public:
	//------------------------------------------------
	// Constructors and destructors
	//------------------------------------------------
	SerialManager();
	~SerialManager();


	//------------------------------------------------
	// Methods
	//------------------------------------------------
	// Initialize serial port
	bool Init(const char* device);

	// Close serial connection
	void CloseSerial();

	// Send ON/OFF
	bool SetPinStat(unsigned short int index, bool val);
	bool SetPinStat(unsigned short int row, unsigned short int col, bool val);

	// Ask for all states and parse
	bool GetPinStat(PinGrid& grid);

	// Serial IO
	ssize_t WriteLine(const char* line);
	ssize_t WriteLine(const std::string& line);
	ssize_t ReadLine(char* buffer, size_t maxlen);
	std::string ReadLine();


	private:
	int fd;
};
