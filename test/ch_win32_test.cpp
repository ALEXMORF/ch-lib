#include "../ch_win32.h"
#include <stdlib.h>

static bool gAppIsDone;

LRESULT CALLBACK
Win32WindowCallback(HWND Window, UINT Message,
                    WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message)
    {
        case WM_CLOSE:
        case WM_QUIT:
        {
            gAppIsDone = true;
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool KeyIsDown = (LParam & (1 << 31)) == 0;
            bool KeyWasDown = (LParam & (1 << 30)) != 0;
            bool AltIsDown = (LParam & (1 << 29)) != 0;
            
            if (KeyIsDown != KeyWasDown)
            {
                if (KeyIsDown && AltIsDown && WParam == VK_RETURN)
                {
                    ch::Win32ToggleFullscreen(Window);
                }
                
                if (AltIsDown && WParam == VK_F4)
                {
                    gAppIsDone = true;
                }
            }
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

int main()
{
    HWND Window = ch::InitWindow(800, 600, "Example", "Example Class",
                                 Win32WindowCallback);
    if (!Window)
    {
        printf("Failed to open window\n");
        return -1;
    }
    
    HDC WindowDC = GetDC(Window);
    
    while (!gAppIsDone)
    {
        MSG Message = {};
        while (PeekMessageA(&Message, Window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
        // fill framebuffer
        int Width, Height;
        ch::Win32GetClientDimension(Window, &Width, &Height);
        uint32_t *BackBuffer = (uint32_t *)calloc(Width*Height, sizeof(uint32_t));
        for (int PixelI = 0; PixelI < Width*Height; ++PixelI)
        {
            BackBuffer[PixelI] = 0x20202020;
        }
        ch::Win32BlitBufferToScreen(WindowDC, BackBuffer, Width, Height);
        free(BackBuffer);
        
        Sleep(10);
    }
    
    return 0;
}