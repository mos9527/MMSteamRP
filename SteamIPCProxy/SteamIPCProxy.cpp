#include <windows.h> 

#include <thread>
#include <vector>
#include <queue>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <optional>

#include <steam_api.h>
#include <isteamfriends.h>
#pragma comment(lib, "steam_api64")

#define PIPENAME "\\\\.\\pipe\\SteamIPCProxyPipe"
#define BUFSIZE 512
#define ASSERT(X,Y) if (!(X)) { \
    MessageBox(NULL,TEXT("Assertion Error:\n" __FILE__ ":" __FUNCSIG__ ":\n" Y),TEXT("SteamIPCProxy"),MB_ICONSTOP); \
    return -1; \
}
#define LOG std::cout << "[SteamIPCProxy] "
#define PIPE_EVENT_SETRP     0x00000000
#define PIPE_EVENT_KEEPALIVE 0x00000001
#define PIPE_EVENT_TERMINATE 0xFFFFFFFF

#define KEEPALIVE_TIMEOUT    10

struct PipeEvent {	
	uint32_t eventType;
	char eventMessage[BUFSIZE - sizeof(uint32_t)]; // Align to avoid padding
	PipeEvent() {
		memset(eventMessage, 0, BUFSIZE - sizeof(uint32_t));
	}
	void SetMessage(const char* src) {
		memcpy(eventMessage, src, strlen(src));
	}
};
class PipeEventQueue : public std::queue<PipeEvent> {
	/* A thread-blocking queue, implemented w/ CV & optionals*/
public:
	void push(PipeEvent& evt) {
		std::lock_guard<std::mutex> lock(guard);
		__super::push(evt);
		signal.notify_one();
	}
	
	std::optional<PipeEvent> pop() {
		/* Pops a PipeEvent object from queue. Timeouts in 1s*/
		std::unique_lock<std::mutex> lock(guard);
		while (__super::empty()) {
			auto status = signal.wait_for(lock,std::chrono::seconds(1));
			if (status == std::cv_status::timeout) return {};
		}
		auto value = __super::front();
		__super::pop();	
		return value;
	}
private:
	std::mutex guard;
	std::condition_variable signal;
};
PipeEventQueue pipeEvents;
int main()
{
	SetConsoleOutputCP(65001);
	SetEnvironmentVariable(L"SteamAppId", L"440");
	// Override AppId at startup.
	// SteamAPI would look for the ids in environs if steam_appid.txt is not found
	ASSERT(
		SteamAPI_Init(),
		"Cannot initialize Steamworks API!"
	);
	std::vector<std::thread> connections;
	std::thread listenerThread = std::thread([&connections]() {
		/* Listen for pipe connections forever, and handles them via new threads.*/
		while (1) {
			auto hPipe = CreateNamedPipe(
				TEXT(PIPENAME),
				PIPE_ACCESS_INBOUND,      // read only access 
				PIPE_TYPE_MESSAGE |       // message type pipe 
				PIPE_READMODE_BYTE |      // message-read mode 
				PIPE_WAIT,                // blocking mode 
				PIPE_UNLIMITED_INSTANCES, // max. instances  
				BUFSIZE,                  // output buffer size 
				BUFSIZE,                  // input buffer size 
				0,                        // client time-out 
				NULL);                    // default security attribute   
			ASSERT(hPipe != INVALID_HANDLE_VALUE, "Cannot create pipe. Exiting...");			
			BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
			if (fConnected)
			{
				connections.push_back(std::thread([hPipe]() {
					while (1) {
						PipeEvent event;
						memset(&event, 0, sizeof(PipeEvent));
						BOOL fSuccess = ReadFile(
							hPipe,        // handle to pipe 
							&event,        // buffer to receive data 
							sizeof(PipeEvent),    // size of buffer 
							NULL,         // number of bytes read 
							NULL);        // not overlapped I/O 						
						if (!fSuccess)
						{							
							LOG << "[SteamIPCProxy] " << "ReadFile on pipe 0x" << std::hex << hPipe << " failed! Terminating.";
							// For now, the proxy will be killed once any connection falls apart
							// Would there be a case where multiple conncetions should be actually maintained?
							event.eventType = PIPE_EVENT_TERMINATE;
							pipeEvents.push(event);
							return -1;
						}
						pipeEvents.push(event);
						// Copy the event into our message queue and let main thread process it.
					}
					}));
			}
			else
				CloseHandle(hPipe);
		}
		});
	PipeEvent evt;
	evt.eventType = PIPE_EVENT_SETRP;
	evt.SetMessage(u8"Standby");
	pipeEvents.push(evt);
	auto tKeepalive = std::chrono::steady_clock::now();
	while (1) {
		/* Work for main thread!*/
		// 1. Poll events and interface Steam via SteamWorks APIs
		auto maybe_evt = pipeEvents.pop();
		if (maybe_evt.has_value()) {
			auto &evt = maybe_evt.value();
			switch (evt.eventType)
			{
			case PIPE_EVENT_SETRP:
				LOG << "Updating rich presense: " << evt.eventMessage << '\n';
				SteamFriends()->SetRichPresence("steam_display", "#TF_RichPresence_State_PlayingGeneric");
				ASSERT(
					SteamFriends()->SetRichPresence("currentmap", evt.eventMessage),
					"Cannot set Steam Rich Presence info! Check steam_appid.txt and/or whether Steam is running normally. \n The program will now exit."
				);
				break;
			case PIPE_EVENT_KEEPALIVE:
				// Reset the timer with these packets
				tKeepalive = std::chrono::steady_clock::now();
				break;
			case PIPE_EVENT_TERMINATE:
				LOG << "Termination request recevied. Exiting...\n";
				goto TERMINATE;
			default:
				break;
			}
		}
		// 2. Check whether we should terminate ourselves based on the keep-alive timer
		if ((std::chrono::steady_clock::now() - tKeepalive) > std::chrono::seconds(KEEPALIVE_TIMEOUT)) {
			goto TERMINATE;
		}
	}
TERMINATE:
	// 1. Terminate all connections (if any)
	for (auto &thread : connections) 
		thread.~thread();	
	// 2. Stop listener
	listenerThread.~thread();
	return 0;
}
