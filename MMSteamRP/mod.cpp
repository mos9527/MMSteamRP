#include <globals.h>
#define DAEMONNAME L"SteamIPCProxy.exe"
#define PIPENAME "\\\\.\\pipe\\SteamIPCProxyPipe"
#define BUFSIZE 512
#define ASSERT_LOG(X,Y) if (!(X)) { \
    LOG(Y); \
    return false; \
}
#define PIPE_EVENT_SETRP     0x00000000
#define PIPE_EVENT_KEEPALIVE 0x00000001
#define PIPE_EVENT_TERMINATE 0xFFFFFFFF
#define RETRY_UNTIL_SUCCESS(X) while(!X()) Sleep(1000);
#define EVENT_MESSAGE_SIZE (BUFSIZE - sizeof(uint32_t))
struct PipeEvent {
    uint32_t eventType;
    char eventMessage[EVENT_MESSAGE_SIZE];
    PipeEvent() {
        memset(eventMessage, 0, EVENT_MESSAGE_SIZE);
    }
    void SetMessage(const char* src) {
        memcpy(eventMessage, src, strlen(src));
    }
};

namespace SteamIPCPipe {
    HANDLE hPipe;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    bool StartDaemon() {        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);        
        ZeroMemory(&pi, sizeof(pi));        
        auto result = CreateProcessW(
            NULL,           // No module name (use command line)
            (LPWSTR)FullPathInDllFolder(std::wstring(DAEMONNAME)).c_str(), // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            (LPWSTR)FullPathInDllFolder(std::wstring()).c_str(), // Parent directory
            &si,            // Pointer to STARTUPINFO structure
            &pi);           // Pointer to PROCESS_INFORMATION structure            
        ASSERT_LOG(result, L"Failed to create process!");    
        LOG(L"Daemon summoned.");
        return true;
    }
    bool InitPipe() {
        ASSERT_LOG(pi.hProcess, L"Daemon process not yet started!");
        hPipe = CreateFile(
            TEXT(PIPENAME),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        ); 
        
        return hPipe != INVALID_HANDLE_VALUE;
    }
    bool IsPipeValid() {
        return hPipe != NULL && hPipe != INVALID_HANDLE_VALUE;
    }
    bool Write(PipeEvent &evt) {
        ASSERT_LOG(IsPipeValid(), L"Pipe not yet created or is not valid!");   
        auto result = WriteFile(
            hPipe,
            &evt,
            sizeof(PipeEvent),
            NULL,
            NULL
        );
        if (!result) {
            LOG(L"Failed to wrtie to pipe! SteamIPCProxy maybe down. Further RP settings will be disabled.");
            hPipe = NULL;
            ZeroMemory(&pi, sizeof(pi));
            return false;
        }
        return true;
    }
    bool WriteRichPresence(const char* buf) {
        PipeEvent evt;
        evt.eventType = PIPE_EVENT_SETRP;
        evt.SetMessage(buf);
        return Write(evt);
    }
    bool WriteKeepAlive() {
        PipeEvent evt;
        evt.eventType = PIPE_EVENT_KEEPALIVE;
        return Write(evt);
    }
    bool WriteTerminate() {
        PipeEvent evt;
        evt.eventType = PIPE_EVENT_TERMINATE;
        return Write(evt);
    }
    void InitAll() {
        RETRY_UNTIL_SUCCESS(StartDaemon);
        RETRY_UNTIL_SUCCESS(InitPipe);
        WriteRichPresence(DEFAULT_RP_MESSAGE);
    }
}
// Signatures
SIG_SCAN
(
    sigChangeGameState,
    0x1402C9660,
    "\x4C\x8B\xC2\x41\xB9\x00\x00\x00\x00\x48\x8B\x15\x00\x00\x00\x00\x48\x8B\x52\x10\xE9\x00\x00\x00\x00",
    "xxxxx????xxx????xxxxx????"
)
#define WITH_STATE(X) if (strcmp(state, X) == 0)
HOOK(INT64, __fastcall, _ChangeGameState, sigChangeGameState(), INT64* a, const char* state) {
	INT64 result = original_ChangeGameState(a, state);
    WITH_STATE("ADV_LOGO") { /* Game started */
        if (!SteamIPCPipe::IsPipeValid()) SteamIPCPipe::InitAll();
    }
    WITH_STATE("PV POST PROCESS TASK") { /* Game begins with any gamemode */
        char buf[EVENT_MESSAGE_SIZE] = { 0 };
        // TODO : Add config support and use libfmt instead
        // Or any other serializers that will allow us to put tokens
        // in arbitary positions. Something like:
        // {title} - {musician} / {difficulty} / {gamemode}
        snprintf(
            buf,
            EVENT_MESSAGE_SIZE,
            "%s - %s/%s - %s - %s",
            DEFAULT_RP_MESSAGE,
            PVWaitScreenInfo->Name.c_str(),
            PVWaitScreenInfo->Music.c_str(),
            GameModeString(),       // TODO : Add localization support for
            GameDifficultyString()  // these strings
        );
        SteamIPCPipe::WriteRichPresence(buf);
    }
    WITH_STATE("PVsel") { /* Game ends and returns to main menu */
        SteamIPCPipe::WriteRichPresence(DEFAULT_RP_MESSAGE);
    }
	return result;
}
extern "C"
{
    void __declspec(dllexport) Init() {
        INSTALL_HOOK(_ChangeGameState);
        LOG(L"Hooks installed.");
    }

    void __declspec(dllexport) OnFrame(void* swapChain) {
        SteamIPCPipe::IsPipeValid() && SteamIPCPipe::WriteKeepAlive();
    }
}