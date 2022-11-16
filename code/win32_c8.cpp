#include "win32_c8.h"

static char HasKilledProcs = 0;
static char IsZenMode = 0;

// NOTE(liam): As this is the Default callback for Windows' message loop and it handles 
LRESULT CALLBACK WindowClassCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0;

	static BOOL QuitMessageBoxIsShowing = FALSE;

	switch (Message)
	{
		case WM_TRAYICON:
		{
			if (!IsZenMode && (LParam == WM_LBUTTONDOWN))
			{
				IsZenMode = 1;
				UpdateTrayIcon(ZenIcon);
				
			}
			else if (IsZenMode && (LParam == WM_LBUTTONDOWN))
			{
				IsZenMode = 0;
				HasKilledProcs = 0;
				UpdateTrayIcon(BaseIcon);
			}
			else if (!QuitMessageBoxIsShowing && (LParam == WM_RBUTTONDOWN || LParam == WM_MBUTTONDOWN))
			{
				QuitMessageBoxIsShowing = TRUE;

				if (MessageBoxW(Window, L"Quit Consentr8?", L"Are you sure?", MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND) == IDYES)
				{
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

int SetTrayIcon(HICON Icon)
{
	TrayNotifyIconData.hIcon = Icon;
    if (Shell_NotifyIconW(NIM_ADD, &TrayNotifyIconData) == FALSE)
	{
		MessageBox(NULL, L"Failed to register systray icon!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

	return 0;
}

int UpdateTrayIcon(HICON Icon)
{
	TrayNotifyIconData.hIcon = Icon;
    if (Shell_NotifyIconW(NIM_MODIFY, &TrayNotifyIconData) == FALSE)
	{
		MessageBox(NULL, L"Failed to register systray icon!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

	return(0);
}

void Concentrate()
{
	if (!IsZenMode) return;
	if (HasKilledProcs) return;
	KillProc(L"Teams.exe");
	KillProc(L"Slack.exe");
	HasKilledProcs = 1;	
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

	if ((NtSuspendProcess = (_NtSuspendProcess)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtSuspendProcess")) == NULL)
	{
		MessageBox(NULL, L"Unable to locate the NtSuspendProcess procedure in the ntdll.dll module. This is an internal Windows function that Consentr8 needs, and it looks like Microsoft have removed it in your version of Windows. Unfortunately, Consentr8 is not yet supported on your system.", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

	if ((NtResumeProcess = (_NtResumeProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtResumeProcess")) == NULL)
	{
		MessageBox(NULL, L"Unable to locate the NtResumeProcess procedure in the ntdll.dll module. This is an internal Windows function that Consentr8 needs, and it looks like Microsoft have removed it in your version of Windows. Unfortunately, Consentr8 is not yet supported on your system.", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
	}

	if ((HungWindowFromGhostWindow = (_HungWindowFromGhostWindow)GetProcAddress(GetModuleHandleW(L"user32.dll"), "HungWindowFromGhostWindow")) == NULL)
	{
		MessageBox(NULL, L"Unable to locate the HungWindowFromGhostWindow procedure in the user32.dll module. This is an internal Windows function that Consentr8 needs, and it looks like Microsoft have removed it in your version of Windows. Unfortunately, Consentr8 is not yet supported on your system.", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
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

    BaseIcon = LoadIcon(Instance, MAKEINTRESOURCE(IDI_ICON1));
    if (!BaseIcon)
    {
        MessageBox(NULL, L"Failed to load icon resource!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
    }

	ZenIcon = LoadIcon(Instance, MAKEINTRESOURCE(IDI_ICON2));
	if (!ZenIcon)
    {
        MessageBox(NULL, L"Failed to load icon resource!", L"Consentr8 Error", MB_OK | MB_ICONERROR);
		return(E_FAIL);
    }

	
	SetTrayIcon(BaseIcon);
    MSG SysTrayWindowMessage = { 0 };
	
    while (AppShouldRun)
    {
        while (PeekMessage(&SysTrayWindowMessage, SystrayWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&SysTrayWindowMessage);
        }
		Concentrate();
		Sleep(1000);
    }

    return(S_OK);
}