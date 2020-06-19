#include "../ch_d3d12.h"
#include "../ch_win32.h"

//
//
// tiny engine, demonstrating ch_d3d12 API

#define BACKBUFFER_COUNT 2
struct tiny_engine
{
    ch::gpu_context Context;
    ch::descriptor_arena RTVArena;
    ch::texture BackBuffers[BACKBUFFER_COUNT];
    IDXGISwapChain3 *SwapChain;
    
    ch::pso RasterPso;
    ID3D12Resource *VB;
    
    int Width;
    int Height;
    bool IsInitialized;
    
    void Run(HWND Window);
    void HandleResize(HWND Window);
};

void tiny_engine::HandleResize(HWND Window)
{
    int CurrentWidth, CurrentHeight;
    ch::Win32GetClientDimension(Window, &CurrentWidth, &CurrentHeight);
    if ((CurrentWidth != 0 && CurrentHeight != 0) && // isn't hidden
        CurrentWidth != Width || CurrentHeight != Height)
    {
        Width = CurrentWidth;
        Height = CurrentHeight;
        
        Context.FlushFramesInFlight();
        
        for (int BI = 0; BI < BACKBUFFER_COUNT; ++BI)
        {
            BackBuffers[BI].Handle->Release();
        }
        
        CH_DXOP(SwapChain->ResizeBuffers(BACKBUFFER_COUNT, 0, 0,
                                         DXGI_FORMAT_R8G8B8A8_UNORM, 0));
        
        for (int BI = 0; BI < BACKBUFFER_COUNT; ++BI)
        {
            ID3D12Resource *Buffer = 0;
            SwapChain->GetBuffer(BI, IID_PPV_ARGS(&Buffer));
            
            ch::descriptor PrevRTV = BackBuffers[BI].RTV;
            BackBuffers[BI] = ch::WrapTexture(Buffer, D3D12_RESOURCE_STATE_PRESENT);
            BackBuffers[BI].RTV = PrevRTV;
            Context.Device->CreateRenderTargetView(BackBuffers[BI].Handle, 
                                                   0, BackBuffers[BI].RTV.CPUHandle);
        }
    }
}

void tiny_engine::Run(HWND Window)
{
    if (!IsInitialized)
    {
        ID3D12Debug *Debug = 0;
        CH_DXOP(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
        Debug->EnableDebugLayer();
        
        ID3D12Device *D = 0;
        CH_DXOP(D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D)));
        
        Context = ch::InitGPUContext(D, BACKBUFFER_COUNT);
        SwapChain = ch::CreateSwapChain(Context.CmdQueue, Window, BACKBUFFER_COUNT);
        ch::Win32GetClientDimension(Window, &Width, &Height);
        
        RTVArena = ch::InitDescriptorArena(D, 100, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (int BI = 0; BI < BACKBUFFER_COUNT; ++BI)
        {
            ID3D12Resource *Buffer = 0;
            CH_DXOP(SwapChain->GetBuffer(BI, IID_PPV_ARGS(&Buffer)));
            BackBuffers[BI] = ch::WrapTexture(Buffer, D3D12_RESOURCE_STATE_PRESENT);
            
            BackBuffers[BI].RTV = RTVArena.PushDescriptor();
            D->CreateRenderTargetView(Buffer, 0, BackBuffers[BI].RTV.CPUHandle);
        }
        
        D3D12_INPUT_ELEMENT_DESC InputElems[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        char Error[256] = {};
        
        RasterPso = ch::InitGraphicsPSO(D, "../shaders/raster_shader.hlsl", 
                                        "VS", "vs_5_1", 
                                        "PS", "ps_5_1",
                                        InputElems, 2, Error, sizeof(Error));
        
        float Vertices[] = {
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
        };
        
        VB = ch::InitBuffer(D, sizeof(Vertices), D3D12_HEAP_TYPE_UPLOAD, // yolo
                            D3D12_RESOURCE_STATE_GENERIC_READ,
                            D3D12_RESOURCE_FLAG_NONE);
        ch::Upload(VB, Vertices, sizeof(Vertices));
        
        IsInitialized = true;
    }
    
    HandleResize(Window);
    
    UINT CurrBackbufferIndex = SwapChain->GetCurrentBackBufferIndex();
    ch::texture *BackBuffer = BackBuffers + CurrBackbufferIndex;
    
    Context.Reset(CurrBackbufferIndex);
    ID3D12GraphicsCommandList *CmdList = Context.CmdList;
    ID3D12CommandQueue *CmdQueue = Context.CmdQueue;
    
    Context.TransitionBarrier(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    Context.FlushBarriers();
    
    float ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    CmdList->ClearRenderTargetView(BackBuffer->RTV.CPUHandle, ClearColor, 0, 0);
    
    D3D12_VIEWPORT Viewport = {};
    Viewport.Width = (float)Width;
    Viewport.Height = (float)Height;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    D3D12_RECT ScissorRect = {};
    ScissorRect.right = (LONG)Width;
    ScissorRect.bottom = (LONG)Height;
    CmdList->RSSetViewports(1, &Viewport);
    CmdList->RSSetScissorRects(1, &ScissorRect);
    
    CmdList->OMSetRenderTargets(1, &BackBuffer->RTV.CPUHandle, TRUE, 0);
    CmdList->SetPipelineState(RasterPso.Handle);
    CmdList->SetGraphicsRootSignature(RasterPso.RootSignature);
    D3D12_VERTEX_BUFFER_VIEW  VBView = {};
    VBView.BufferLocation = VB->GetGPUVirtualAddress();
    VBView.SizeInBytes = 3 * 6*sizeof(float);
    VBView.StrideInBytes = 6*sizeof(float);
    CmdList->IASetVertexBuffers(0, 1, &VBView);
    CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CmdList->DrawInstanced(3, 1, 0, 0);
    
    Context.TransitionBarrier(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
    Context.FlushBarriers();
    
    CH_DXOP(CmdList->Close());
    
    ID3D12CommandList *CmdLists[] = {CmdList};
    CmdQueue->ExecuteCommandLists(1, CmdLists);
    SwapChain->Present(1, 0);
    
    UINT NextBackbufferIndex = SwapChain->GetCurrentBackBufferIndex();
    Context.WaitForGpu(NextBackbufferIndex);
}

//
//
// win32 platform code

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
    HWND Window = ch::Win32InitWindow(800, 600, "Example", "Example Class",
                                      Win32WindowCallback);
    if (!Window)
    {
        printf("Failed to open window\n");
        return -1;
    }
    
    tiny_engine Engine = {};
    
    while (!gAppIsDone)
    {
        MSG Message = {};
        while (PeekMessageA(&Message, Window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
        Engine.Run(Window);
        Sleep(1);
    }
    
    return 0;
}