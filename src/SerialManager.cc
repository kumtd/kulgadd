////////////////////////////////////////////////////////////////////////////////
///
///   SerialManager.cc
///
///   The definition of SerialManager class.
///
///   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
///            Kyungmin Lee (  railroad@korea.ac.kr)
///            Changi Jeong (  jchg3876@korea.ac.kr)
///
////////////////////////////////////////////////////////////////////////////////



///-----------------------------------------------------------------------------
/// Headers
///-----------------------------------------------------------------------------
#include "global.hh"
#include "SerialManager.hh"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>



///-----------------------------------------------------------------------------
/// Constructors and destructors
///-----------------------------------------------------------------------------
///---------------------------------------------------------
/// Constructors
///---------------------------------------------------------
SerialManager::SerialManager() : serialDev("/dev/ttyACM0")
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager] Constructed." << std::endl;
	}
}

SerialManager::SerialManager(const std::string& dev) : serialDev(dev)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager] Constructed." << std::endl;
	}
}


///---------------------------------------------------------
/// Destructor
///---------------------------------------------------------
SerialManager::~SerialManager()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager] Destructed." << std::endl;
	}


	isConnected = false;

	if ( monitorThread . joinable() )
	{
		monitorThread . join();
	}

	if ( serialFd != -1 )
	{
		close(serialFd);
	}
}



///-----------------------------------------------------------------------------
/// Public methods
///-----------------------------------------------------------------------------
///---------------------------------------------------------
/// Connect serial
///---------------------------------------------------------
bool SerialManager::Connect()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager::Connect] Try to connect" << std::endl;
	}

	serialFd = open(serialDev . c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if ( serialFd < 0 )
	{
		std::cerr << "[kulgadd::SerialManager::Connect] Failed to open " << serialDev << ": " << strerror(errno) << "\n";
		return false;
	}

	if ( ! SetupSerialPort(serialFd) )
	{
		close(serialFd);
		serialFd = -1;
		return false;
	}

	isConnected = true;
	monitorThread = std::thread(&SerialManager::MonitorSerial, this);

	return true;
}


///---------------------------------------------------------
/// Write a line to the serial
///---------------------------------------------------------
bool SerialManager::WriteLine(const std::string& line)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager::WriteLine] Writing a line \"" << line << "\" to the serial" << std::endl;
	}

	if ( serialFd < 0 ) return false;

	std::string msg = line + "\n";
	ssize_t written = write(serialFd, msg . c_str(), msg . size());
	return written == static_cast<ssize_t>(msg . size());
//	return true;
}


///---------------------------------------------------------
/// Get buffered response
///---------------------------------------------------------
std::optional<std::string> SerialManager::GetBufferedResponse()
{
	std::lock_guard<std::mutex> lock(bufferMutex);
	if ( responseBuffer . empty() )
	{
		if ( gVerbose > 1 )
		{
			std::cout << "[kulgadd::SerialManager::GetBufferedResponse] Buffer is empty." << std::endl;
		}

		return std::nullopt;
	}

	std::string result = responseBuffer;
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager::GetBufferedResponse] Response: " << result << std::endl;
	}
	responseBuffer . clear();

	return result;
}


///---------------------------------------------------------
/// Send ON/OFF
///---------------------------------------------------------
bool SerialManager::SetPinStat(unsigned short int index, bool val)
{
	//--------------------------------------
	// Try set
	//--------------------------------------
	if ( serialFd < 0 ) return false;
	char cmd[32];

	// ASCII version
	snprintf(cmd, sizeof(cmd), "%s %d", val ? "ON" : "OFF", index);

	// JSON version
//	snprintf(cmd, );

	if ( WriteLine(cmd) < 0 ) return false;

	//--------------------------------------
	// Check response
	//--------------------------------------
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	auto response = GetBufferedResponse();
	if ( response )
	{
		char expected[32];
		snprintf(expected, sizeof(expected), "turning %s %d", val ? "ON" : "OFF", index);
		return response . has_value() && response . value() == expected;
	}
	else
	{
		std::cerr << "[kulgadd::SerialManager::SetPinStat] Try to set stat of pin " << index << " to " << val << ", but no response." << std::endl;
		return false;
	}
}



///-----------------------------------------------------------------------------
/// Private methods
///-----------------------------------------------------------------------------
///---------------------------------------------------------
/// Monitor serial's return
///---------------------------------------------------------
void SerialManager::MonitorSerial()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager::MonitorSerial] Starting serial monitoring" << std::endl;
	}

	char buf[256];
	std::string line;

	while ( isConnected )
	{
		ssize_t len = read(serialFd, buf, sizeof(buf));
		if ( len > 0 )
		{
			for ( ssize_t i = 0; i < len; ++i )
			{
				if ( buf[i] == '\n' )
				{
					{
						std::lock_guard<std::mutex> lock(bufferMutex);
						responseBuffer = line;
					}
					if ( gVerbose > 0 ) std::cout << "[kulgadd::SerialManager::Monitor] " << line << std::endl;
					gServer -> Deliver(line);
					line . clear();

					//------------------
					// Behavior
					//------------------
					auto responseOpt = GetBufferedResponse();
					const std::string& responseStr = responseOpt . value();
					nlohmann::json j = nlohmann::json::parse(responseStr);

					// Is it okay?
					if ( j["ok"] . get<int>() == 1 )
					{
						// When pinstat asked
						if ( j . contains("pins") )
						{
							std::vector<int> pins = j["pins"] . get<std::vector<int>>();
							for ( unsigned short int pin = 0; pin < pins. size(); pin++ )
							{
								if      ( pins[pin] == 1 ) gGrid -> Set(pin, true );
								else if ( pins[pin] == 0 ) gGrid -> Set(pin, false);
							}
							gServer -> BroadcastState();
						}

						// When ON command
						if ( j . contains("cmd") && j["cmd"] . is_string() && j["cmd"] == "ON" )
						{
							if ( j . contains("results") && j["results"] . is_array() )
							{
								for ( const auto& item : j["results"] )
								{
									int pin = item . value("pin", -1);
									gGrid -> Set(pin, true);
								}
							}
						}

						// When OFF command
						if ( j . contains("cmd") && j["cmd"] . is_string() && j["cmd"] == "OFF" )
						{
							if ( j . contains("results") && j["results"] . is_array() )
							{
								for ( const auto& item : j["results"] )
								{
									int pin = item . value("pin", -1);
									gGrid -> Set(pin, false);
								}
							}
						}
					}
				}
				else if (buf[i] != '\n')
				{
					line += buf[i];
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}


///---------------------------------------------------------
/// Serial port setup
///---------------------------------------------------------
bool SerialManager::SetupSerialPort(int fd)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kulgadd::SerialManager::SetupSerialPort] Setup serial port" << std::endl;
	}


	struct termios tty;
	if ( tcgetattr(fd, &tty) != 0 )
	{
		std::cerr << "[kulgadd::SerialManager::SetupSerialPort] tcgetattr error: " << strerror(errno) << "\n";
		return false;
	}

	cfmakeraw(&tty);
	// Baud rate
	cfsetspeed(&tty, B115200);

	// Local connection, activate read
	tty . c_cflag |= (CLOCAL | CREAD);

	// deactivate RTS/CTS stream control
	tty . c_cflag &= ~CRTSCTS;
	tty . c_cc[VMIN] = 0;

	// 0.1 sec timeout
	tty . c_cc[VTIME] = 1;


	if ( tcsetattr(fd, TCSANOW, &tty) != 0 )
	{
		std::cerr << "[kulgadd::SerialManager::SetupSerialPort] tcsetattr error: " << strerror(errno) << "\n";
		return false;
	}

	return true;
}
