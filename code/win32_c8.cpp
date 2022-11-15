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

#define WM_TRAYICON (WM_USER + 1)

NOTIFYICONDATA TrayNotifyIconData;
// Used to enforce single instance
HANDLE Mutex;

BOOL AppShouldRun = TRUE;

// Forward decs
void KillProc(WCHAR *exeName);

// The WindowProc (callback) for WinMain's WindowClass.
// Basically the system tray does nothing except lets the user know that it's running.
// If the user clicks the tray icon in any way it will ask if they want to exit the app.
LRESULT CALLBACK WindowClassCallback(_In_ HWND Window, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
	LRESULT Result = 0;

	static BOOL QuitMessageBoxIsShowing = FALSE;

	switch (Message)
	{
		case WM_TRAYICON:
		{
			if (!QuitMessageBoxIsShowing && (LParam == WM_LBUTTONDOWN || LParam == WM_RBUTTONDOWN || LParam == WM_MBUTTONDOWN))
			{
				QuitMessageBoxIsShowing = TRUE;

				if (MessageBoxW(Window, L"Quit Consentr8?", L"Are you sure?", MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL) == IDYES)
				{
					KillProc(L"Teams.exe");
					Shell_NotifyIconW(NIM_DELETE, &TrayNotifyIconData);

					AppShouldRun = FALSE;

					PostQuitMessage(0);
				}
				else
				{
					QuitMessageBoxIsShowing = FALSE;
				}
			}
			break;
		}
		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);

			break;
		}
	}
	return(Result);
}

void KillProc(WCHAR *exeName)
{
    HANDLE SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 Entry;
    Entry.dwSize = sizeof(Entry);
    BOOL Result = Process32First(SnapshotHandle, &Entry);

    while (Result)
    {
        if (wcscmp(Entry.szExeFile, exeName) == 0)
        {
            HANDLE ProcessHandle = OpenProcess(PROCESS_TERMINATE, 0, (DWORD) Entry.th32ProcessID);
            if (ProcessHandle)
            {
                TerminateProcess(ProcessHandle, 9);
                CloseHandle(ProcessHandle);
            }
        }
        Result = Process32Next(SnapshotHandle, &Entry);
    }
    CloseHandle(SnapshotHandle);
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowWindowFlag)
{
    Mutex = CreateMutex(NULL, FALSE, L"Consentr8");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, L"An instance of Consentr8 us already running", L"Consentr8 Error", MB_OK | MB_ICONERROR);
        return(ERROR_ALREADY_EXISTS);
    }

    WNDCLASS SysTrayWindowClass = { 0 };
	SysTrayWindowClass.style         = CS_HREDRAW | CS_VREDRAW;
	SysTrayWindowClass.hInstance     = Instance;
	SysTrayWindowClass.lpszClassName = L"Consentr8_Systray_WindowClass";
	SysTrayWindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
	SysTrayWindowClass.lpfnWndProc   = WindowClassCallback;

	if (RegisterClass(&SysTrayWindowClass) == 0)
	{
		MessageBox(NULL, L"Failed to register WindowClass!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

	HWND SystrayWindow = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		SysTrayWindowClass.lpszClassName,
		L"Consentr8_Systray_Window",
		WS_ICONIC,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		Instance,
		0);

	if (SystrayWindow == 0)
	{
		MessageBox(NULL, L"Failed to create SystrayWindow!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

    TrayNotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	TrayNotifyIconData.hWnd = SystrayWindow;
	TrayNotifyIconData.uID = 20401984;
	TrayNotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	TrayNotifyIconData.uCallbackMessage = WM_TRAYICON;

	wcscpy_s(TrayNotifyIconData.szTip, _countof(TrayNotifyIconData.szTip), L"Consentr8 v1.0.0");

    HICON TrayIcon = (HICON)LoadImage(0, L"trayicon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if (!TrayIcon)
    {
        MessageBox(NULL, L"Failed to load systray icon resource!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
    }

    TrayNotifyIconData.hIcon = TrayIcon;
    if (Shell_NotifyIconW(NIM_ADD, &TrayNotifyIconData) == FALSE)
	{
		MessageBoxW(NULL, L"Failed to register systray icon!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

    MSG        SysTrayWindowMessage                 = { 0 };
	DWORD      PreviouslySuspendedProcessID			= 0;
	HWND	   PreviouslySuspendedWnd				= 0;
	wchar_t    PreviouslySuspendedProcessText[256]  = { 0 };
	HANDLE     ProcessHandle                        = 0;

    while (AppShouldRun)
    {
        while (PeekMessage(&SysTrayWindowMessage, SystrayWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&SysTrayWindowMessage);
        }
    }

    return(S_OK);
}