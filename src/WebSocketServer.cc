////////////////////////////////////////////////////////////////////////////////
//
//   WebSocketServer.cc
//
//   The definition of WebSocketServer class.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//            Changi Jeong (  jchg3876@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "global.hh"
#include "WebSocketServer.hh"

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>
#include <libwebsockets.h>



using json = nlohmann::json;



//------------------------------------------------------------------------------
// Anonymous namespace
//------------------------------------------------------------------------------
namespace
{
	static WebSocketServer* g_instance = nullptr;

	static int websocket_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
	{
		switch ( reason )
		{
			// New connection?
			case LWS_CALLBACK_ESTABLISHED:
				g_instance -> OnClientConnected(wsi);
				break;

			// Someone disconnect?
			case LWS_CALLBACK_CLOSED:
				g_instance -> OnClientDisconnected(wsi);
				break;

			// A client has sent any message?
			case LWS_CALLBACK_RECEIVE:
				g_instance -> OnClientMessage(wsi, std::string((const char*)in, len));
				break;

			case LWS_CALLBACK_SERVER_WRITEABLE:
				break;

			default:
				break;
		}
		
		return 0;
	}

	static struct lws_protocols protocols[] =
	{
		{
			"ws",                // Protocol name. Must be same as frontend's code
			websocket_callback,  // Callback function pointer
			0,
			4096
		},
		{ nullptr, nullptr, 0, 0 }  // Protocol list ends with null.
	};
} 



//------------------------------------------------------------------------------
// Constructors and destructor
//------------------------------------------------------------------------------
//----------------------------------------------------------
// Constructor
//----------------------------------------------------------
WebSocketServer::WebSocketServer(SerialManager* serial_, PinGrid* grid_) : serial(serial_), pinGrid(grid_)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer] Object constructed." << std::endl;
	}
}


//----------------------------------------------------------
// Destructor
//----------------------------------------------------------
WebSocketServer::~WebSocketServer()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer] Object destructed." << std::endl;
	}

	Stop();
}



//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------
//----------------------------------------------------------
// Start method
//----------------------------------------------------------
bool WebSocketServer::Start(int port)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::Start] Starting WS server." << std::endl;
	}

	if ( isRunning ) return false;
	isRunning = true;
	g_instance = this;

	serverThread = std::thread(&WebSocketServer::ServerLoop, this);
//	serialMonitorThread = std::thread(&WebSocketServer::MonitorSerial, this);
	return true;
}


//----------------------------------------------------------
// Stop method
//----------------------------------------------------------
void WebSocketServer::Stop()
{
	if ( !isRunning ) return;
	isRunning = false;

	if ( context                          ) lws_cancel_service(context);
	if ( serverThread . joinable()        ) serverThread . join();
//	if ( serialMonitorThread . joinable() ) serialMonitorThread . join();

	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::Stop] WS server stopped." << std::endl;
	}
}



//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
//----------------------------------------------------------
// Server loop
//----------------------------------------------------------
void WebSocketServer::ServerLoop()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::ServerLoop] Executed." << std::endl;
	}

	//--------------------------------------
	// Create websocket context
	//--------------------------------------
	lws_context_creation_info info = {};
	info . port = 3001;
	info . protocols = protocols;
	info . gid = -1;
	info . uid = -1;

	context = lws_create_context(&info);

	//--------------------------------------
	// Check if websocket context well created
	//--------------------------------------
	if ( ! context )
	{
		std::cerr << "[kumtdd::WebSocketServer::ServerLoop] Failed to create lws context" << std::endl;
		return;
	}

	//--------------------------------------
	// Enter the loop and wait for websocket connection
	//--------------------------------------
	while ( isRunning )
	{
		lws_service(context, 50);
	}

	//--------------------------------------
	// Finalize
	//--------------------------------------
	lws_context_destroy(context);
	context = nullptr;
}


//----------------------------------------------------------
// On client connected
//----------------------------------------------------------
void WebSocketServer::OnClientConnected(lws* wsi)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::OnClientConnected] Greetings to the new client and send the current stat" << std::endl;
	}

	std::lock_guard<std::mutex> lock(clientMutex);
	clients . insert(wsi);
	SendToClient(wsi, pinGrid -> ToJSONString());
}


//----------------------------------------------------------
// On client disconnected
//----------------------------------------------------------
void WebSocketServer::OnClientDisconnected(lws* wsi)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::OnClientConnected] Goodbye my client" << std::endl;
	}

	std::lock_guard<std::mutex> lock(clientMutex);
	clients . erase(wsi);
}


//----------------------------------------------------------
// On client message
//----------------------------------------------------------
void WebSocketServer::OnClientMessage(lws* wsi, const std::string& msg)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::OnClientMessage] A message received from a client: " << msg << std::endl;
	}

	try
	{
		auto j = json::parse(msg);
		if ( j . contains("cmd") && j["cmd"] == "set" )
		{
			int ch = j["ch"];
			bool val = j["val"];
			if ( ch >= 0 && ch < 256 )
			{
				if ( serial -> SetPinStat(ch, val) )
				{
					pinGrid -> Set(ch, val);
					BroadcastState();
				}
				else
				{
					std::cerr << "[kumtdd::WebSocketServer::OnClientMessage] Fail to set pin stat via serial" << std::endl;
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "JSON parse error: " << e . what() << std::endl;
	}
}


//----------------------------------------------------------
// Broadcast state
//----------------------------------------------------------
void WebSocketServer::BroadcastState()
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::BroadcastState] Broadcasting" << std::endl;
	}

	std::lock_guard<std::mutex> lock(clientMutex);
	std::string state = pinGrid -> ToJSONString();
	for ( lws* wsi : clients )
	{
		SendToClient(wsi, state);
	}

	return;
}


//----------------------------------------------------------
// Send to client
//----------------------------------------------------------
void WebSocketServer::SendToClient(lws* wsi, const std::string& msg)
{
	//--------------------------------------
	// Debugging message
	//--------------------------------------
	if ( gVerbose > 1 )
	{
		std::cout << "[kumtdd::WebSocketServer::SendToClient] Sending message" << std::endl;
	}

	size_t len = msg . size();
	std::vector<unsigned char> buf(LWS_PRE + len);
	std::memcpy(&buf[LWS_PRE], msg . data(), len);
	lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
}
