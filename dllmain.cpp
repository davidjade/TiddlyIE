// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "TiddlyIEBHO_i.h"
#include "dllmain.h"

CTiddlyIEBHOModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);
	}

	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
