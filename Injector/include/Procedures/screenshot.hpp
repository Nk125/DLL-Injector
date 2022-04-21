#include <atlbase.h>
#include <atlconv.h>
#include <regex>
#include <string>
#include <vector>
#include <windows.h>
#define SSPATH "screenshot.bmp"
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")

void MAIN(HINSTANCE hModule) {
    DWORD wrt;
    std::string fp;
    std::wstring tmp;
    tmp.resize(MAX_PATH);

    GetTempPathW(MAX_PATH, &tmp[0]);

    fp = std::string{ ATL::CW2A(tmp.data()).m_psz } + "\\" + SSPATH;
    fp = std::regex_replace(fp, std::regex(R"(\\\\)"), "\\");

    HANDLE f = CreateFileA(fp.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    STARTUPINFOA st;
    PROCESS_INFORMATION pi;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE* bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;

    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&st, sizeof(st));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    
    WriteFile(f, &bfHeader, sizeof(BITMAPFILEHEADER), &wrt, 0);
    WriteFile(f, &biHeader, sizeof(BITMAPINFOHEADER), &wrt, 0);
    WriteFile(f, bBits, cbBits, &wrt, 0);

    DeleteObject(hBitmap);

    CloseHandle(f);

    CreateProcessA("explorer.exe",
        const_cast<LPSTR>(std::string{ " " + fp}.c_str()),
        NULL, NULL, 0, 0, NULL, NULL, &st, &pi);
    // ImageView_Fullscreen can't be called with rundll32, as this need admin perms

    return;
}