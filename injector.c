#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int InjectDLL(DWORD pid, const char* dllPath);
DWORD GetPIDByProcessName(const char* processName);
void ErrorMessage(const char* message, ...);

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    ErrorMessage("help: %s <PID> <DLL PATH>", argv[0]);
    return 1;
  }

  DWORD pid = GetPIDByProcessName(argv[1]);
  if (pid == 0)
  {
    ErrorMessage("Invalid PID. Please enter a valid number.");
    return 1;
  }

  const char* dllPath = argv[2];
  return InjectDLL(pid, dllPath);
}

int InjectDLL(DWORD pid, const char* dllPath)
{
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProcess)
  {
    ErrorMessage("Failed to open process. Error: %lu", GetLastError());
    return 1;
  }

  LPVOID remotePath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
  if (!remotePath)
  {
    ErrorMessage("VirtualAllocEx failed.");
    CloseHandle(hProcess);
    return 1;
  }
  WriteProcessMemory(hProcess, remotePath, dllPath, strlen(dllPath) + 1, NULL);

  LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

  HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
  if (!hThread)
  {
    ErrorMessage("CreateRemoteThread failed.");
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 1;
  }

  WaitForSingleObject(hThread, INFINITE);
  VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
  CloseHandle(hThread);
  CloseHandle(hProcess);

  printf("DLL injected successfully!\n");
  return 0;
}

DWORD GetPIDByProcessName(const char* processName)
{
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
  {
    printf("Failed to create snapshot.\n");
    return 0;
  }

  if (Process32First(hSnapshot, &pe32))
  {
    do
    {
      if (_stricmp(pe32.szExeFile, processName) == 0)
      {
        DWORD pid = pe32.th32ProcessID;
        CloseHandle(hSnapshot);
        return pid;
      }
    } while (Process32Next(hSnapshot, &pe32));
  }

  CloseHandle(hSnapshot);
  return 0;
}

void ErrorMessage(const char* fmtMessage, ...)
{
  va_list args;
  va_start(args, fmtMessage);
  va_list args_copy;
  va_copy(args_copy, args);

  int len = vsnprintf(NULL, 0, fmtMessage, args_copy);
  va_end(args_copy);

  int size = len + 1;
  char* rawMessage = (char*)malloc(size * sizeof(char));
  if (rawMessage == NULL)
  {
    va_end(args);
    return;
  }

  vsnprintf(rawMessage, size, fmtMessage, args);
  va_end(args);
  MessageBoxA(NULL, rawMessage, "ERROR", MB_OK | MB_ICONERROR);
  free(rawMessage);
  return;
}
