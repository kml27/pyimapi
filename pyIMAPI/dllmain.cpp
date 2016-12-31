// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//printf("process attach thread %d", GetCurrentThread());
		//Work on COM thread detection and errors, support Multiprocessing not GIL
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		//printf("initialized com");

	case DLL_THREAD_ATTACH:

		//printf("thread attach thread %d", GetCurrentThread());
		break;
	
	case DLL_PROCESS_DETACH:
		//printf("process detach thread %d", GetCurrentThread());

		CoUninitialize();
		//printf("uninit com");
	case DLL_THREAD_DETACH:
		//printf("thread detach thread %d", GetCurrentThread());

		break;
	}
	return TRUE;
}

