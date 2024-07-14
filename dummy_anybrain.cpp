#include <Windows.h>
#include <memory.h>

#include "utils.h"

typedef int(*AnybrainVoidFunction)(void);
typedef int(*AnybrainSetCredentialsFunction)(const char*, const char*);
typedef int(*AnybrainSetUserIdFunction)(const char*, size_t);
typedef enum
{
  UNINITIALIZED,
  INITIALIZED,
  STOPPED,
  RUNNING,
  PAUSED
} AnybrainStatus;

DWORD WINAPI show_message(LPVOID param);
void set_engine_status(AnybrainStatus status);
DWORD WINAPI auto_pause_resume(LPVOID param);
void auto_pause_resume_initialize(void);
extern "C" __declspec(dllexport) int AnybrainStartSDK(void);
extern "C" __declspec(dllexport) int AnybrainPauseSDK(void);
extern "C" __declspec(dllexport) int AnybrainResumeSDK(void);
extern "C" __declspec(dllexport) int AnybrainStopSDK(void);
extern "C" __declspec(dllexport) int AnybrainSetCredentials(const char* arg0, const char* arg1);
extern "C" __declspec(dllexport) int AnybrainSetUserId(const char* arg0, size_t arg1);
void main(void);
int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved);

AnybrainVoidFunction           original_AnybrainStartSDK       = NULL;
AnybrainVoidFunction           original_AnybrainPauseSDK       = NULL;
AnybrainVoidFunction           original_AnybrainResumeSDK      = NULL;
AnybrainVoidFunction           original_AnybrainStopSDK        = NULL;
AnybrainSetCredentialsFunction original_AnybrainSetCredentials = NULL;
AnybrainSetUserIdFunction      original_AnybrainSetUserId      = NULL;

HWND window_handle = NULL;
AnybrainStatus engine_status = UNINITIALIZED;
bool auto_pause_resume_initialized = false;

DWORD WINAPI show_message(LPVOID param)
{
  SHOW_INFO((char*)param);

  return 0;
}

void set_engine_status(AnybrainStatus status)
{
  engine_status = status;

  if (window_handle != NULL)
  {
    switch (engine_status)
    {
    case UNINITIALIZED:
      SetWindowTextA(window_handle, "Engine status: UNINITIALIZED");
      break;
    case INITIALIZED:
      SetWindowTextA(window_handle, "Engine status: INITIALIZED");
      break;
    case STOPPED:
      SetWindowTextA(window_handle, "Engine status: STOPPED");
      break;
    case RUNNING:
      SetWindowTextA(window_handle, "Engine status: RUNNING");
      break;
    case PAUSED:
      SetWindowTextA(window_handle, "Engine status: PAUSED");
      break;
    default:
      SetWindowTextA(window_handle, "Engine status: UNKNOWN");
    }
  }
}

DWORD WINAPI auto_pause_resume(LPVOID param)
{
  DWORD time_before_pause = read_ini_int("config", "timeBeforePause", 30000);
  DWORD time_before_resume = read_ini_int("config", "timeBeforeResume", 60000);

  while (engine_status != STOPPED)
  {
    Sleep(time_before_pause);
    AnybrainPauseSDK();
    Sleep(time_before_resume);
    AnybrainResumeSDK();
  }

  return 0;
}

void auto_pause_resume_initialize(void)
{
  if (auto_pause_resume_initialized)
  {
    return;
  }

  if (read_ini_int("config", "autoPauseResume", 0) == 1)
  {
    CreateThread(0, 0, auto_pause_resume, NULL, 0, 0);

    window_handle = get_toplevel_window();

    auto_pause_resume_initialized = true;
  }
}

extern "C" __declspec(dllexport) int AnybrainStartSDK(void)
{
  if (original_AnybrainStartSDK == NULL)
  {
    set_engine_status(INITIALIZED);

    return 0;
  }

  int result = original_AnybrainStartSDK();

  if (result == 0)
  {
    set_engine_status(INITIALIZED);
  }

  return result;
}

extern "C" __declspec(dllexport) int AnybrainPauseSDK(void)
{
  if (engine_status == PAUSED)
  {
    return 0;
  }

  if (original_AnybrainPauseSDK == NULL)
  {
    set_engine_status(PAUSED);

    return 0;
  }

  int result = original_AnybrainPauseSDK();

  if (result == 0)
  {
    set_engine_status(PAUSED);
  }

  return result;
}

extern "C" __declspec(dllexport) int AnybrainResumeSDK(void)
{
  if (engine_status == RUNNING)
  {
    return 0;
  }

  if (original_AnybrainResumeSDK == NULL)
  {
    set_engine_status(RUNNING);

    return 0;
  }

  int result = original_AnybrainResumeSDK();

  if (result == 0)
  {
    set_engine_status(RUNNING);
  }

  return result;
}

extern "C" __declspec(dllexport) int AnybrainStopSDK(void)
{
  if (original_AnybrainStopSDK == NULL)
  {
    set_engine_status(STOPPED);

    return 0;
  }

  int result = original_AnybrainStopSDK();

  if (result == 0)
  {
    set_engine_status(STOPPED);
  }

  return result;
}

extern "C" __declspec(dllexport) int AnybrainSetCredentials(const char* arg0, const char* arg1)
{
  if (original_AnybrainSetCredentials == NULL)
  {
    return 0;
  }

  if (read_ini_int("config", "fakeAnybrainSetCredentials", 0) == 1)
  {
    char* fake_arg0 = (char*)malloc(MAX_PATH);
    int read_arg0 = read_ini_str("config", "fakeAnybrainSetCredentialsArg0", arg0, fake_arg0, MAX_PATH);

    if (read_arg0 == -1)
    {
      SHOW_ERROR("Could not read fakeAnybrainSetCredentialsArg0 from INI file");
    }

    char* fake_arg1 = (char*)malloc(MAX_PATH);
    int read_arg1 = read_ini_str("config", "fakeAnybrainSetCredentialsArg1", arg1, fake_arg1, MAX_PATH);

    if (read_arg1 == -1)
    {
      SHOW_ERROR("Could not read fakeAnybrainSetCredentialsArg1 from INI file");
    }

    int result = original_AnybrainSetCredentials(fake_arg0, fake_arg1);

    free(fake_arg1);
    free(fake_arg0);

    return result;
  }

  return original_AnybrainSetCredentials(arg0, arg1);
}

extern "C" __declspec(dllexport) int AnybrainSetUserId(const char* arg0, size_t arg1)
{
  if (original_AnybrainSetUserId == NULL)
  {
    auto_pause_resume_initialize();

    return 0;
  }

  if (read_ini_int("config", "fakeAnybrainSetUserId", 0) == 1)
  {
    char* fake_arg0 = (char*)malloc(MAX_PATH);
    int read_arg0 = read_ini_str("config", "fakeAnybrainSetUserIdArg0", arg0, fake_arg0, MAX_PATH);

    if (read_arg0 == -1)
    {
      SHOW_ERROR("Could not read fakeAnybrainSetUserIdArg0 from INI file");
    }

    int result = original_AnybrainSetUserId(fake_arg0, strlen(fake_arg0));

    free(fake_arg0);

    if (result == 0)
    {
      auto_pause_resume_initialize();
    }

    return result;
  }

  int result = original_AnybrainSetUserId(arg0, arg1);

  if (result == 0)
  {
    auto_pause_resume_initialize();
  }

  return result;
}

void main(void)
{
  LPSTR exe_path = (LPSTR)malloc(MAX_PATH);
  DWORD exe_path_size = get_executable_path(exe_path);
  size_t slash_index = 0;

  for (slash_index = exe_path_size; slash_index >= 0; --slash_index)
  {
    if (exe_path[slash_index] == '\\')
    {
      break;
    }

    exe_path[slash_index] = '\0';
  }

  if (slash_index + 32 >= MAX_PATH)
  {
    SHOW_ERROR("Path is too long");

    return;
  }

  exe_path[slash_index + 1] = 's';
  exe_path[slash_index + 2] = 'o';
  exe_path[slash_index + 3] = 'u';
  exe_path[slash_index + 4] = 'n';
  exe_path[slash_index + 5] = 'd';
  exe_path[slash_index + 6] = 's';
  exe_path[slash_index + 7] = '\\';
  exe_path[slash_index + 8] = 'w';
  exe_path[slash_index + 9] = 'o';
  exe_path[slash_index + 10] = 'o';
  exe_path[slash_index + 11] = 'b';
  exe_path[slash_index + 12] = 'y';
  exe_path[slash_index + 13] = '_';
  exe_path[slash_index + 14] = 's';
  exe_path[slash_index + 15] = 'o';
  exe_path[slash_index + 16] = 'u';
  exe_path[slash_index + 17] = 'n';
  exe_path[slash_index + 18] = 'd';
  exe_path[slash_index + 19] = '_';
  exe_path[slash_index + 20] = 'o';
  exe_path[slash_index + 21] = 'r';
  exe_path[slash_index + 22] = 'i';
  exe_path[slash_index + 23] = 'g';
  exe_path[slash_index + 24] = 'i';
  exe_path[slash_index + 25] = 'n';
  exe_path[slash_index + 26] = 'a';
  exe_path[slash_index + 27] = 'l';
  exe_path[slash_index + 28] = '.';
  exe_path[slash_index + 29] = 'b';
  exe_path[slash_index + 30] = 'n';
  exe_path[slash_index + 31] = 'k';
  exe_path[slash_index + 32] = '\0';

  HMODULE wooby_dll = LoadLibraryA(exe_path);

  if (wooby_dll == NULL)
  {
    SHOW_ERROR("Could not load the original DLL");

    return;
  }

  free(exe_path);

  if (read_ini_int("config", "skipAnybrainStartSDK", 0) != 1)
  {
    original_AnybrainStartSDK = (AnybrainVoidFunction)GetProcAddress(wooby_dll, "AnybrainStartSDK");
  }

  if (read_ini_int("config", "skipAnybrainPauseSDK", 0) != 1)
  {
    original_AnybrainPauseSDK = (AnybrainVoidFunction)GetProcAddress(wooby_dll, "AnybrainPauseSDK");
  }

  if (read_ini_int("config", "skipAnybrainResumeSDK", 0) != 1)
  {
    original_AnybrainResumeSDK = (AnybrainVoidFunction)GetProcAddress(wooby_dll, "AnybrainResumeSDK");
  }

  if (read_ini_int("config", "skipAnybrainStopSDK", 0) != 1)
  {
    original_AnybrainStopSDK = (AnybrainVoidFunction)GetProcAddress(wooby_dll, "AnybrainStopSDK");
  }

  if (read_ini_int("config", "skipAnybrainSetCredentials", 0) != 1)
  {
    original_AnybrainSetCredentials = (AnybrainSetCredentialsFunction)GetProcAddress(wooby_dll, "AnybrainSetCredentials");
  }

  if (read_ini_int("config", "skipAnybrainSetUserId", 0) != 1)
  {
    original_AnybrainSetUserId = (AnybrainSetUserIdFunction)GetProcAddress(wooby_dll, "AnybrainSetUserId");
  }

  if (read_ini_int("config", "proxyAnybrainHTTPRequests", 0) == 1)
  {
    LONG64 wooby_original_address = 0;
    LONG64 wooby_original_size = 0;

    get_executable_image_data(&wooby_original_address, &wooby_original_size, "wooby_sound_original.bnk");

    if (wooby_original_address == 0 || wooby_original_size == 0)
    {
      SHOW_ERROR("Could not find module wooby_sound_original.bnk in memory");

      goto proxy_anybrain_http_requests_end;
    }

    char* pattern = (char*)"deeep-storage.anybrain.gg";
    LONG64 pattern_size = 25;
    LONG64 address = search_memory(wooby_original_address, wooby_original_size, pattern, pattern_size);

    if (address == -1)
    {
      SHOW_ERROR("Could not find deeep-storage.anybrain.gg pattern in wooby_sound_original.bnk");

      goto proxy_anybrain_http_requests_end;
    }

    LONG64 wooby_original_section_address = 0;
    LONG64 wooby_original_section_size = 0;

    get_module_section_for_address(&wooby_original_section_address, &wooby_original_section_size, address, "wooby_sound_original.bnk");

    if (wooby_original_section_address == 0 || wooby_original_section_size == 0)
    {
      SHOW_ERROR("Could not find URL section in wooby_sound_original.dll");

      goto proxy_anybrain_http_requests_end;
    }

    char patch[26] = { 0 };

    char* host = (char*)malloc(MAX_PATH);
    int read_host = read_ini_str("config", "proxyAnybrainHTTPRequestsHost", "deeep-storage.anybrain.gg", host, MAX_PATH);

    if (read_host == -1)
    {
      SHOW_ERROR("Could not read proxyAnybrainHTTPRequestsHost from INI file");
    }

    size_t host_size = strlen(host);

    if (host_size > 25)
    {
      SHOW_ERROR("Maximum size for proxyAnybrainHTTPRequestsHost is 25");
    }
    else
    {
      for (size_t i = 0; i < host_size; ++i)
      {
        patch[i] = host[i];
      }
    }

    free(host);

    DWORD old_protection;

    VirtualProtect((LPVOID)wooby_original_section_address, wooby_original_section_size, PAGE_EXECUTE_READWRITE, &old_protection);

    write_memory(address, patch, sizeof(patch));

    VirtualProtect((LPVOID)wooby_original_section_address, wooby_original_section_size, old_protection, &old_protection);

    proxy_anybrain_http_requests_end:
      do {} while (0);
  }

  CreateThread(0, 0, show_message, "Dummy Anybrain loaded!", 0, 0);

  return;
}

int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
	{
    main();
	}

	return 1;
}
