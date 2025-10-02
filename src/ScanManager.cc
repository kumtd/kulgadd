////////////////////////////////////////////////////////////////////////////////
///
///   ScanManager.cc
///
///   The definition of ScanManager class.
///
///   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
///            Kyungmin Lee (  railroad@korea.ac.kr)
///            Changi Jeong (  jchg3876@korea.ac.kr)
///
////////////////////////////////////////////////////////////////////////////////



///-----------------------------------------------------------------------------
/// Headers
///-----------------------------------------------------------------------------
#include "ScanManager.hh"
#include <nlohmann/json.hpp>



///-----------------------------------------------------------------------------
/// JSON namespace
///-----------------------------------------------------------------------------
using json = nlohmann::json;



///----------------------------------------------------------------------------
/// Constructor / Destructor
///----------------------------------------------------------------------------
ScanManager::ScanManager()
	: pid_(-1), running(false), stdout_fd(-1), stderr_fd(-1)
{
}


ScanManager::~ScanManager()
{
	try
	{
		Stop();
	}
	catch (...)
	{
	}

	CleanupReaders();
}



///----------------------------------------------------------------------------
/// Public methods
///----------------------------------------------------------------------------
///---------------------------------------------------------
/// Set command
///---------------------------------------------------------
void ScanManager::SetCMD(const std::string& cmd)
{
    command = cmd;
}


///---------------------------------------------------------
/// Start process
///---------------------------------------------------------
bool ScanManager::Start()
{
	if ( running . load() ) return false;

	int outpipe[2];
	int errpipe[2];
	if ( pipe(outpipe) == -1 ) return false;
	if ( pipe(errpipe) == -1 )
	{
		close(outpipe[0]);
		close(outpipe[1]);
		return false;
	}

	pid_t p = fork();
	if ( p < 0 )
	{
		close(outpipe[0]);
		close(outpipe[1]);
		close(errpipe[0]);
		close(errpipe[1]);
		return false;
	}

	if ( p == 0 )
	{
		// Child
		dup2(outpipe[1], STDOUT_FILENO);
		dup2(errpipe[1], STDERR_FILENO);

		close(outpipe[0]);
		close(outpipe[1]);
		close(errpipe[0]);
		close(errpipe[1]);

		setpgid(0, 0);

		execl("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr);
		_exit(127); // exec failed
	}

	// Parent
	pid_ = p;
	close(outpipe[1]);
	close(errpipe[1]);

	stdout_fd = outpipe[0];
	stderr_fd = errpipe[0];

	running . store(true);

	stdout_thread = std::thread(&ScanManager::ReaderThread, this,
		                        stdout_fd, std::ref(stdout_buf), std::ref(stdout_mutex));
	stderr_thread = std::thread(&ScanManager::ReaderThread, this,
		                        stderr_fd, std::ref(stderr_buf), std::ref(stderr_mutex));

	return true;
}


///---------------------------------------------------------
/// Get running status
///---------------------------------------------------------
bool ScanManager::IsRunning()
{
	if ( !running . load() || pid_ <= 0 ) return false;

	int status = 0;
	pid_t r = waitpid(pid_, &status, WNOHANG);
	if ( r == 0 )
	{
		return true;
	}
	else if ( r == pid_ )
	{
		std::lock_guard<std::mutex> lk(status_mutex);
		exit_status = status;
		running . store(false);
		CleanupReaders();
		return false;
	}
	else
	{
		running . store(false);
		CleanupReaders();
		return false;
	}
}


///---------------------------------------------------------
/// Kill the process
///---------------------------------------------------------
void ScanManager::Stop(int timeout_ms)
{
	if ( pid_ <= 0 ) return;
	if ( !running . load() ) return;
	
	kill(-pid_, SIGTERM);

	auto start = std::chrono::steady_clock::now();
	while ( IsRunning() )
	{
		auto now = std::chrono::steady_clock::now();
		if ( std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_ms )
		{
			kill(-pid_, SIGKILL);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	IsRunning();
}


///---------------------------------------------------------
/// Get exit status
///---------------------------------------------------------
std::optional<int> ScanManager::GetExitStatus()
{
	std::lock_guard<std::mutex> lk(status_mutex);
	return exit_status;
}

std::string ScanManager::ReadStdOut()
{
	std::lock_guard<std::mutex> lk(stdout_mutex);
	return stdout_buf;
}

std::string ScanManager::ReadStdErr()
{
	std::lock_guard<std::mutex> lk(stderr_mutex);
	return stderr_buf;
}

void ScanManager::Wait()
{
	if ( pid_ <= 0 ) return;
	int status = 0;
	if ( waitpid(pid_, &status, 0) == pid_ )
	{
		std::lock_guard<std::mutex> lk(status_mutex);
		exit_status = status;
		running . store(false);
		CleanupReaders();
	}
}


///------------------------------------------------
/// JSON handling: stat to JSON
///------------------------------------------------
std::string ScanManager::ToJSONString() const
{
	json j;
	if ( running ) j["scan"] = 1;
	else           j["scan"] = 0;
	return j . dump();
}


//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ScanManager::CleanupReaders()
{
	if ( stdout_fd != -1)
	{
		::close(stdout_fd);
		stdout_fd = -1;
	}
	if ( stderr_fd != -1 )
	{
		::close(stderr_fd);
		stderr_fd = -1;
	}
	if ( stdout_thread . joinable() ) stdout_thread . join();
	if ( stderr_thread . joinable() ) stderr_thread . join();
}


void ScanManager::ReaderThread(int fd, std::string& outBuf, std::mutex& bufMutex)
{
	if ( fd < 0 ) return;
	constexpr size_t BUF_SZ = 4096;
	std::vector<char> buf(BUF_SZ);
	ssize_t n;
	while ( (n = read(fd, buf.data(), BUF_SZ)) > 0 )
	{
		std::lock_guard<std::mutex> lk(bufMutex);
		outBuf . append(buf.data(), static_cast<size_t>(n));
	}
}
