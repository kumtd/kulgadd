////////////////////////////////////////////////////////////////////////////////
///
///   WebSocketServer.hh
///
///   Web socket server class.
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
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include <string>

#include "global.hh"
#include "SerialManager.hh"
#include "PinGrid.hh"



///-----------------------------------------------------------------------------
/// Forward declaration of structs
/// These are to save session data.
///-----------------------------------------------------------------------------
struct lws;
struct lws_context;



///-----------------------------------------------------------------------------
/// Class declaration
///-----------------------------------------------------------------------------
class WebSocketServer
{
	public:
	//----------------------------------------------------------
	// Constructors & destructor
	//----------------------------------------------------------
	WebSocketServer(SerialManager* serial, PinGrid* grid);
	~WebSocketServer();


	//----------------------------------------------------------
	// Public methods
	//----------------------------------------------------------
	bool Start(int port = 3000);
	void Stop();

	void ServerLoop();

	void OnClientConnected(lws* wsi);
	void OnClientDisconnected(lws* wsi);
	void OnClientMessage(lws* wsi, const std::string& msg);
	void BroadcastState();
	void SendToClient(lws* wsi, const std::string& msg);
	void Deliver(const std::string& msg);
	bool IsIPAllowed(const char* ipStr);


	private:
	//----------------------------------------------------------
	// Private members
	//----------------------------------------------------------
	std::thread serverThread;
	std::atomic<bool> isRunning{false};
	lws_context* context = nullptr;
	
	SerialManager* serial;
	PinGrid* pinGrid;

	std::mutex clientMutex;
	std::unordered_set<lws*> clients;  // active client connections
};
