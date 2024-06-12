#include "kernel64.h"

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	//case DLL_PROCESS_ATTACH: return tools::kernel::driver::get_instance().load();
	//case DLL_PROCESS_DETACH: return tools::kernel::driver::get_instance().unload();
	default:
		auto& inst = tools::windows::kernel::driver::get_instance();
		return true;
	}
}