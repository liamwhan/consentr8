#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif



#include <Windows.h>
#include <shellapi.h>
#include <Psapi.h>
#include <stdio.h>
#include <process.h>
#include <Tlhelp32.h>
#include <stdlib.h>
#include "../resource.h"

#define WM_TRAYICON (WM_USER + 1)


// INFO(liam): These are undocumented Win32 API functions so Windows could remove
// them at any time, so we must check for their existence before we use them.
typedef LONG(NTAPI* _NtSuspendProcess) (IN HANDLE ProcessHandle);
typedef LONG(NTAPI* _NtResumeProcess) (IN HANDLE ProcessHandle);
typedef HWND(NTAPI* _HungWindowFromGhostWindow) (IN HWND GhostWindowHandle);

_NtSuspendProcess NtSuspendProcess;
_NtResumeProcess NtResumeProcess;
_HungWindowFromGhostWindow HungWindowFromGhostWindow;

NOTIFYICONDATA TrayNotifyIconData;

HICON BaseIcon;
HICON ZenIcon;

//NOTE(liam): Used to enforce single instance
HANDLE Mutex;

BOOL AppShouldRun = TRUE;

// Forward decs
void KillProc(WCHAR *exeName);
LRESULT CALLBACK WindowClassCallback(_In_ HWND Window, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowWindowFlag);
int UpdateTrayIcon(HICON Icon);
int SetTrayIcon(HICON Icon);
void Concentrate();
