////////////////////////////////////////////////////////////////////////////////
//
//   SerialManager.cc
//
//   The definition of SerialManager class.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "SerialManager.hh"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <sstream>



//------------------------------------------------------------------------------
// Constructors and destructors
//------------------------------------------------------------------------------
//------------------------------------------------
// Default constructor
//------------------------------------------------
SerialManager::SerialManager() : fd(-1)
{
}


//------------------------------------------------
// Default destructor
//------------------------------------------------
SerialManager::~SerialManager()
{
	CloseSerial();
}


//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------
//------------------------------------------------
// Initialize serial port
//------------------------------------------------
bool SerialManager::Init(const char* device)
{
	// Open serial
	fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);

	if ( fd < 0 )
	{
		perror("serial open failed");
		return false;
	}

	struct termios tty;
	if ( tcgetattr(fd, &tty) != 0 )
	{
		perror("tcgetattr failed");
		CloseSerial();
		return false;
	}

	// Baud rate is 115200 in general.
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	// tty configuration
	tty . c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty . c_iflag &= ~IGNBRK;
	tty . c_lflag = 0;
	tty . c_oflag = 0;
	tty . c_cc[VMIN] = 1;
	tty . c_cc[VTIME] = 1;

	tty . c_iflag &= ~(IXON | IXOFF | IXANY);
	tty . c_cflag |= (CLOCAL | CREAD);
	tty . c_cflag &= ~(PARENB | PARODD);
	tty . c_cflag &= ~CSTOPB;
	tty . c_cflag &= ~CRTSCTS;

	// Try to apply configuration
	if ( tcsetattr(fd, TCSANOW, &tty) != 0 )
	{
		perror("tcsetattr failed");
		CloseSerial();
		return false;
	}

	return true;
}


//------------------------------------------------
// Close serial connection
//------------------------------------------------
void SerialManager::CloseSerial()
{
	if ( fd >= 0 )
	{
		close(fd);
		fd = -1;
	}
}


//------------------------------------------------
// Send ON/OFF
//------------------------------------------------
bool SerialManager::SetCellStat(int cell, bool val)
{
	if ( fd < 0 ) return false;
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "%s %d", val ? "ON" : "OFF", cell);
	if ( WriteLine(cmd) < 0 ) return false;

	char response[64];
	ssize_t n = ReadLine(response, sizeof(response));
	if ( n <= 0 ) return false;

	// Expecting response like "turning ON n" or "turning OFF n"
	char expected[32];
	snprintf(expected, sizeof(expected), "turning %s %d", val ? "ON" : "OFF", cell);
	
	return (strncmp(response, expected, strlen(expected)) == 0);
}


//------------------------------------------------
// Ask for state and parse
//------------------------------------------------
bool SerialManager::GetCellStat(CellGrid& grid)
{
	if ( fd < 0 ) return false;

	const char* cmd = "PINSTAT ALL";
	if ( WriteLine(cmd) < 0 ) return false;

	// Get response
	char response[1024];
	ssize_t n = ReadLine(response, sizeof(response));
	std::cout << n << std::endl;
	if ( n <= 0 ) return false;

	std::istringstream iss(response);
	for ( int i = 0; i < 256; i++ )
	{
		int state;
		if ( !(iss >> state) ) return false;
		grid . Set(i, state != 0);
	}


	return true;
}


//------------------------------------------------
// Serial IO
//------------------------------------------------
// Write line
ssize_t SerialManager::WriteLine(const char* line)
{
	if ( fd < 0 ) return -1;
	size_t len = strlen(line);
	ssize_t ret = write(fd, line, len);
	if ( ret != (ssize_t) len ) return -1;
	ret = write(fd, "\n", 1);
	if ( ret != 1 ) return -1;
	return len + 1;
}

ssize_t SerialManager::WriteLine(const std::string& line)
{
	return WriteLine(line . c_str());
}

// Read line
ssize_t SerialManager::ReadLine(char* buffer, size_t maxlen)
{
	if ( fd < 0 || maxlen == 0 ) return -1;
	size_t pos = 0;
	while (pos < maxlen - 1)
	{
		std::cout << "hello " << pos << std::endl;
		char c;
		ssize_t n = read(fd, &c, 1);
		if ( n <= 0    ) break;
		if ( c == '\n' ) break;
		buffer[pos++] = c;
	}
	buffer[pos] = '\0';
	return pos;
}

std::string SerialManager::ReadLine()
{
	char buffer[512];
	ssize_t len = ReadLine(buffer, sizeof(buffer));
	if ( len <= 0 ) return "";
	return std::string(buffer, len);
}
