#pragma once
#include <stdint.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ch
{
    static void
        Win32MessageBox(char *Title, UINT Type, char *Fmt, ...)
    {
        va_list Args;
        va_start(Args, Fmt);
        
        char Buf[2048];
        vsnprintf(Buf, sizeof(Buf), Fmt, Args);
        va_end(Args);
        
        MessageBoxA(0, Buf, Title, Type);
    }
    
    static void
        Win32Panic(char *Fmt, ...)
    {
        va_list Args;
        va_start(Args, Fmt);
        
        char Buf[2048];
        vsnprintf(Buf, sizeof(Buf), Fmt, Args);
        va_end(Args);
        
        MessageBoxA(0, Buf, "Panic", MB_OK|MB_ICONERROR);
        
        ExitProcess(1);
    }
    
    static WINDOWPLACEMENT GlobalWindowPosition;
    static void
        Win32ToggleFullscreen(HWND Window)
    {
        DWORD Style = GetWindowLong(Window, GWL_STYLE);
        if (Style & WS_OVERLAPPEDWINDOW)
        {
            MONITORINFO mi = { sizeof(mi) };
            if (GetWindowPlacement(Window, &GlobalWindowPosition) &&
                GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &mi))
            {
                SetWindowLong(Window, GWL_STYLE,
                              Style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(Window, HWND_TOP,
                             mi.rcMonitor.left, mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
        else
        {
            SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(Window, &GlobalWindowPosition);
            SetWindowPos(Window, NULL, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    
    static HWND 
        InitWindow(int ClientWidth, int ClientHeight, 
                   char *Title, char *ClassName, 
                   WNDPROC Win32WindowCallback)
    {
        HMODULE Instance = GetModuleHandleA(0);
        
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_OWNDC;
        WindowClass.lpfnWndProc = Win32WindowCallback;
        WindowClass.hInstance = Instance;
        WindowClass.hCursor = LoadCursorA(0, IDC_ARROW);
        WindowClass.lpszClassName = ClassName;
        
        if (RegisterClassA(&WindowClass))
        {
            // first input client rect
            RECT WindowRect = {};
            WindowRect.left = 0;
            WindowRect.top = 0;
            WindowRect.right = ClientWidth;
            WindowRect.bottom = ClientHeight;
            
            DWORD WindowStyle = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
            
            //find right window size
            RECT WindowSize = {};
            {
                HDC ScreenDeviceContext = GetWindowDC(0);
                
                int ScreenWidth = GetDeviceCaps(ScreenDeviceContext, HORZRES);
                int ScreenHeight = GetDeviceCaps(ScreenDeviceContext, VERTRES);
                
                int WindowLeft = (ScreenWidth - ClientWidth) / 2;
                int WindowTop = (ScreenHeight - ClientHeight) / 2;
                
                WindowSize = {WindowLeft, WindowTop, WindowLeft + ClientWidth, WindowTop + ClientHeight};
                AdjustWindowRect(&WindowSize, WindowStyle, FALSE);
                
                ReleaseDC(0, ScreenDeviceContext);
            }
            
            HWND Window = CreateWindowExA(0,
                                          WindowClass.lpszClassName,
                                          Title,
                                          WindowStyle,
                                          WindowSize.left,
                                          WindowSize.top,
                                          WindowSize.right - WindowSize.left,
                                          WindowSize.bottom - WindowSize.top,
                                          0, 0, Instance, 0);
            
            return Window;
        }
        
        return 0;
    }
    
    static void
        Win32BlitBufferToScreen(HDC WindowDC, void *Buffer, int Width, int Height)
    {
        BITMAPINFO BitmapInfo = {};
        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biWidth = Width;
        BitmapInfo.bmiHeader.biHeight = Height;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        StretchDIBits(WindowDC, 0, 0, Width, Height, 0, 0, Width, Height, 
                      Buffer, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }
    
    static void
        Win32GetClientDimension(HWND Window, int *Width, int *Height)
    {
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        *Width = ClientRect.right - ClientRect.left;
        *Height = ClientRect.bottom - ClientRect.top;
    }
    
    inline uint64_t Win32GetPerformanceFrequency()
    {
        LARGE_INTEGER Result = {};
        QueryPerformanceFrequency(&Result);
        return Result.QuadPart;
    }
    
    inline uint64_t Win32GetPerformanceCounter()
    {
        LARGE_INTEGER Result = {};
        QueryPerformanceCounter(&Result);
        return Result.QuadPart;
    }
};