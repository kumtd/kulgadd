////////////////////////////////////////////////////////////////////////////////
///
///   ScanManager.hh
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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>
#include <sstream>
#include <iostream>
#include <chrono>



///-----------------------------------------------------------------------------
/// Class declaration
///-----------------------------------------------------------------------------
class ScanManager
{
	public:
	ScanManager();
	~ScanManager();

	void SetCMD(const std::string& cmd);

	bool Start();
	bool IsRunning();
	void Stop(int timeout_ms = 2000);
	std::optional<int> GetExitStatus();

	std::string ReadStdOut();
	std::string ReadStdErr();
	void Wait();

	// JSON handling
	std::string ToJSONString() const;


	private:
	void CleanupReaders();
	void ReaderThread(int fd, std::string& outBuf, std::mutex& bufMutex);

	std::string command;
	pid_t pid_;
	std::atomic<bool> running;
	int stdout_fd, stderr_fd;

	std::thread stdout_thread;
	std::thread stderr_thread;
	std::string stdout_buf;
	std::string stderr_buf;
	std::mutex stdout_mutex, stderr_mutex;

	std::mutex status_mutex;
	std::optional<int> exit_status;
};
