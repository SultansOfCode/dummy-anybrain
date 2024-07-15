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
void patch_original_host(void);
void main(void);
int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved);

char original_dll_path[MAX_PATH] = { 0 };
size_t original_dll_path_size = 0;

char original_dll_name[MAX_PATH] = { 0 };
size_t original_dll_name_size = 0;

HMODULE original_dll_handle = NULL;

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
	DWORD time_before_pause = read_ini_int("autoPauseResume", "timeBeforePause", 30000);
	DWORD time_before_resume = read_ini_int("autoPauseResume", "timeBeforeResume", 60000);

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

	if (read_ini_int("autoPauseResume", "enabled", 0) != 1)
	{
		return;
	}

	CreateThread(0, 0, auto_pause_resume, NULL, 0, 0);

	auto_pause_resume_initialized = true;

	if (read_ini_int("debug", "onInstallAutoPauseResume", 0) == 1)
	{
		CreateThread(0, 0, show_message, "Auto pause/resume installed", 0, 0);
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

	if (read_ini_int("fakeCredentials", "enabled", 0) == 1)
	{
		char* fake_arg0 = (char*)malloc(MAX_PATH);
		int read_arg0 = read_ini_str("fakeCredentials", "arg0", arg0, fake_arg0, MAX_PATH);

		if (read_arg0 == -1)
		{
			SHOW_ERROR("Could not read arg0 from fakeCredentials section in INI file");
		}

		char* fake_arg1 = (char*)malloc(MAX_PATH);
		int read_arg1 = read_ini_str("fakeCredentials", "arg1", arg1, fake_arg1, MAX_PATH);

		if (read_arg1 == -1)
		{
			SHOW_ERROR("Could not read arg1 from fakeCredentials section in INI file");
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

	if (read_ini_int("fakeUserId", "enabled", 0) == 1)
	{
		char* fake_arg0 = (char*)malloc(MAX_PATH);
		int read_arg0 = read_ini_str("fakeUserId", "arg0", arg0, fake_arg0, MAX_PATH);

		if (read_arg0 == -1)
		{
			SHOW_ERROR("Could not read arg0 from fakeUserId section in INI file");
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

void patch_original_host(void)
{
	LONG64 original_dll_address = 0;
	LONG64 original_dll_size = 0;

	get_module_image_data(&original_dll_address, &original_dll_size, original_dll_name);

	if (original_dll_address == 0 || original_dll_size == 0)
	{
		SHOW_ERROR("Could not find original DLL module in memory");

		return;
	}

	char original_host[MAX_PATH] = { 0 };
	int read_original_host = read_ini_str("proxyHTTPRequests", "originalHost", "deeep-storage.anybrain.gg", original_host, sizeof(original_host));

	if (read_original_host == -1)
	{
		SHOW_ERROR("Could not read originalHost from INI file");

		return;
	}
	
	LONG64 original_host_size = strlen(original_host);
	LONG64 original_host_address = search_memory(original_dll_address, original_dll_size, original_host, original_host_size);

	if (original_host_address == -1)
	{
		SHOW_ERROR("Could not find original host pattern in original DLL");

		return;
	}

	LONG64 original_dll_section_address = 0;
	LONG64 original_dll_section_size = 0;

	get_module_section_for_address(&original_dll_section_address, &original_dll_section_size, original_host_address, original_dll_name);

	if (original_dll_section_address == 0 || original_dll_section_size == 0)
	{
		SHOW_ERROR("Could not find URL section in original DLL");

		return;
	}

	char patch[26] = { 0 };

	char target_host[MAX_PATH] = { 0 };
	int read_target_host = read_ini_str("proxyHTTPRequests", "targetHost", "deeep-storage.anybrain.gg", target_host, sizeof(target_host));

	if (read_target_host == -1)
	{
		SHOW_ERROR("Could not read targetHost from INI file");
	}

	size_t target_host_size = strlen(target_host);

	if (target_host_size > original_host_size)
	{
		SHOW_ERROR("Target host string is bigger than the original host");

		return;
	}

	DWORD old_protection;

	VirtualProtect((LPVOID)original_dll_section_address, original_dll_section_size, PAGE_EXECUTE_READWRITE, &old_protection);

	write_memory(original_host_address, target_host, target_host_size + 1);

	VirtualProtect((LPVOID)original_dll_section_address, original_dll_section_size, old_protection, &old_protection);

	if (read_ini_int("debug", "onPatchProxyHTTPRequests", 0) == 1)
	{
		CreateThread(0, 0, show_message, "Patched the HTTP requests", 0, 0);
	}
}

void main(void)
{
	if (!init_dll())
	{
		SHOW_ERROR("Could not initialize DLL path");

		return;
	}

	if (!init_ini())
	{
		SHOW_ERROR("Could not initialize INI path");

		return;
	}

	if (!change_path_extension(dll_path, dll_path_size, "dll", original_dll_path))
	{
		SHOW_ERROR("Could not initialize original DLL path");

		return;
	}

	original_dll_path_size = strlen(original_dll_path);

	extract_file_name_data(original_dll_path, original_dll_name, &original_dll_name_size);

	original_dll_handle = LoadLibraryA(original_dll_path);

	if (original_dll_handle == NULL)
	{
		SHOW_ERROR("Could not load the original DLL");

		return;
	}

	if (read_ini_int("skipOriginal", "AnybrainStartSDK", 0) != 1)
	{
		original_AnybrainStartSDK = (AnybrainVoidFunction)GetProcAddress(original_dll_handle, "AnybrainStartSDK");
	}

	if (read_ini_int("skipOriginal", "AnybrainPauseSDK", 0) != 1)
	{
		original_AnybrainPauseSDK = (AnybrainVoidFunction)GetProcAddress(original_dll_handle, "AnybrainPauseSDK");
	}

	if (read_ini_int("skipOriginal", "AnybrainResumeSDK", 0) != 1)
	{
		original_AnybrainResumeSDK = (AnybrainVoidFunction)GetProcAddress(original_dll_handle, "AnybrainResumeSDK");
	}

	if (read_ini_int("skipOriginal", "AnybrainStopSDK", 0) != 1)
	{
		original_AnybrainStopSDK = (AnybrainVoidFunction)GetProcAddress(original_dll_handle, "AnybrainStopSDK");
	}

	if (read_ini_int("skipOriginal", "AnybrainSetCredentials", 0) != 1)
	{
		original_AnybrainSetCredentials = (AnybrainSetCredentialsFunction)GetProcAddress(original_dll_handle, "AnybrainSetCredentials");
	}

	if (read_ini_int("skipOriginal", "AnybrainSetUserId", 0) != 1)
	{
		original_AnybrainSetUserId = (AnybrainSetUserIdFunction)GetProcAddress(original_dll_handle, "AnybrainSetUserId");
	}

	if (read_ini_int("proxyHTTPRequests", "enabled", 0) == 1)
	{
		patch_original_host();
	}

	if (read_ini_int("debug", "onEngineStatusChange", 0) == 1)
	{
		window_handle = get_toplevel_window();
	}

	if (read_ini_int("debug", "onFinishLoading", 0) == 1)
	{
		CreateThread(0, 0, show_message, "Dummy Anybrain loaded!", 0, 0);
	}

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
