////////////////////////////////////////////////////////////////////////////////
///
///   SerialManager.hh
///
///   This class opens serial port, sends commands, and handles response.
///
///   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
///            Kyungmin Lee (  railroad@korea.ac.kr)
///            Changi Jeong (  jchg3876@korea.ac.kr)
///
////////////////////////////////////////////////////////////////////////////////



#pragma once



///-----------------------------------------------------------------------------
/// Headers
///-----------------------------------------------------------------------------
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <optional>

#include "global.hh"



///-----------------------------------------------------------------------------
/// Class declaration
///-----------------------------------------------------------------------------
class SerialManager
{
	public:
	//----------------------------------------------------------
	// Constructors & destructor
	//----------------------------------------------------------
	SerialManager();
	SerialManager(const std::string& dev);
	~SerialManager();


	//----------------------------------------------------------
	// Public methods
	//----------------------------------------------------------
	bool Connect();
	bool WriteLine(const std::string& line);
	std::optional<std::string> GetBufferedResponse();

	// Send ON/OFF
	bool SetPinStat(unsigned short int index, bool val);
	bool SetPinStat(unsigned short int row, unsigned short int col, bool val);


	private:
	//----------------------------------------------------------
	// Private members
	//----------------------------------------------------------
	std::string serialDev;
	int serialFd = -1;
	std::thread monitorThread;
	std::mutex bufferMutex;
	std::string responseBuffer;
	std::atomic<bool> isConnected{false};


	//----------------------------------------------------------
	// Private methods
	//----------------------------------------------------------
	void MonitorSerial();
	bool SetupSerialPort(int fd);
};
