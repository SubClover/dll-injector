#include <windows.h>
#include <stdio.h>

DWORD WINAPI MainDllProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    printf("DLL loaded.\n");
    DisableThreadLibraryCalls(hModule);
    CreateThread(NULL, 0, MainDllProc, NULL, 0, NULL);
    break;
  case DLL_PROCESS_DETACH:
    printf("DLL unloaded.\n");
    break;
  }
  return TRUE;
}

__declspec(dllexport) void hello()
{
  MessageBoxA(NULL, "Hello from DLL!", "DLL Message", MB_OK);
}

DWORD WINAPI MainDllProc(LPVOID lpParam)
{
  while (1)
  {
    printf("running!\n");
    Sleep(1000);
  }
  return 0;
}
