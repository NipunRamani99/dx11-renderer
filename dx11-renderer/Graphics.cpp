#include "Graphics.hpp"
#include <sstream>
#include "dxerr.h"
#include "GraphicsThrowMacros.h"
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "D3DCompiler.lib" )
#include "WindowsConstants.h"
using namespace Microsoft;
namespace dx = DirectX;
// Graphics exception stuff
Graphics::HrException::HrException ( int line, const char* file, HRESULT hr,
                                     std::vector<std::string> infoMsgs ) noexcept
    : Exception ( line, file ), hr ( hr )
{
    // join all info messages with newlines into single string
    for ( const auto& m : infoMsgs )
    {
        info += m;
        info.push_back ( '\n' );
    }
    // remove final newline if exists
    if ( !info.empty () )
    {
        info.pop_back ();
    }
}

const char* Graphics::HrException::what () const noexcept
{
    std::ostringstream oss;
    oss << GetType () << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode () << std::dec << " ("
        << (unsigned long)GetErrorCode () << ")" << std::endl
        << "[Error String] " << GetErrorString () << std::endl
        << "[Description] " << GetErrorDescription () << std::endl
        << GetOriginString ();
    whatBuffer = oss.str ();
    return whatBuffer.c_str ();
}

const char* Graphics::HrException::GetType () const noexcept
{
    return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode () const noexcept
{
    return hr;
}

std::string Graphics::HrException::GetErrorString () const noexcept
{
    return DXGetErrorString ( hr );
}

std::string Graphics::HrException::GetErrorDescription () const noexcept
{
    char buf[512];
    DXGetErrorDescription ( hr, buf, sizeof ( buf ) );
    return buf;
}

const char* Graphics::DeviceRemovedException::GetType () const noexcept
{
    return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::Graphics ( HWND hwnd )
{
    namespace wrl                         = Microsoft::WRL;

    DXGI_SWAP_CHAIN_DESC sd               = {};
    sd.BufferDesc.Width                   = SCREEN_WIDTH;
    sd.BufferDesc.Height                  = SCREEN_HEIGHT;
    sd.BufferDesc.Format                  = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount                        = 1;
    sd.OutputWindow                       = hwnd;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags                              = 0;
    UINT swapCreateFlags                  = 0u;

#ifndef NDEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr;
    // create device and front/back buffers, and swap chain and rendering context
    GFX_THROW_INFO ( D3D11CreateDeviceAndSwapChain ( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                                     D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, nullptr, &pContext ) );

    WRL::ComPtr<ID3D11Resource> pBackBuffer;
    GFX_THROW_INFO ( pSwap->GetBuffer ( 0, __uuidof ( ID3D11Resource ), &pBackBuffer ) );
    GFX_THROW_INFO ( pDevice->CreateRenderTargetView ( pBackBuffer.Get (), nullptr, &pTarget ) );

    // create depth stensil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable              = TRUE;
    dsDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc                = D3D11_COMPARISON_LESS;
    wrl::ComPtr<ID3D11DepthStencilState> pDSState;
    GFX_THROW_INFO ( pDevice->CreateDepthStencilState ( &dsDesc, &pDSState ) );

    // bind depth state
    pContext->OMSetDepthStencilState ( pDSState.Get (), 1u );

    // create depth stensil texture
    wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC depthTexDesc = {};
    depthTexDesc.Width                = SCREEN_WIDTH;
    depthTexDesc.Height               = SCREEN_HEIGHT;
    depthTexDesc.MipLevels            = 1u;
    depthTexDesc.ArraySize            = 1u;
    depthTexDesc.Format               = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.SampleDesc.Count     = 1u;
    depthTexDesc.SampleDesc.Quality   = 0u;
    depthTexDesc.Usage                = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
    GFX_THROW_INFO ( pDevice->CreateTexture2D ( &depthTexDesc, nullptr, &pDepthStencil ) );

    // create view of depth stensil texture
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
    depthViewDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
    depthViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDesc.Texture2D.MipSlice            = 0u;
    GFX_THROW_INFO ( pDevice->CreateDepthStencilView ( pDepthStencil.Get (), &depthViewDesc, &pDSV ) );

    // bind depth stensil view to OM
    pContext->OMSetRenderTargets ( 1u, pTarget.GetAddressOf (), pDSV.Get () );

    // configure viewport
    D3D11_VIEWPORT vp;
    vp.Width    = (float)SCREEN_WIDTH;
    vp.Height   = (float)SCREEN_HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    pContext->RSSetViewports ( 1u, &vp );

    // init imgui d3d impl
    ImGui_ImplDX11_Init ( pDevice.Get (), pContext.Get () );
}

void Graphics::EndFrame ()
{

    if ( imguiEnabled )
    {
        ImGui::Render ();
        ImGui_ImplDX11_RenderDrawData ( ImGui::GetDrawData () );
    }

    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set ();
#endif // !NDEBUG
    if ( FAILED ( hr = pSwap->Present ( 1u, 0u ) ) )
    {
        if ( hr == DXGI_ERROR_DEVICE_REMOVED )
        {
            throw GFX_DEVICE_REMOVED_EXCEPT ( pDevice->GetDeviceRemovedReason () );
        }
        else
        {
            throw GFX_EXCEPT ( hr );
        }
    }
}

void Graphics::BeginFrame ( float red, float green, float blue ) noexcept
{
    const float color[] = { red, green, blue, 1.0f };
    pContext->ClearRenderTargetView ( pTarget.Get (), color );
    pContext->ClearDepthStencilView ( pDSV.Get (), D3D11_CLEAR_DEPTH, 1.0f, 0u );

    if ( imguiEnabled )
    {
        ImGui_ImplDX11_NewFrame ();
        ImGui_ImplWin32_NewFrame ();
        ImGui::NewFrame ();
    }
}

void Graphics::DrawIndexed ( UINT count ) noexcept ( !IS_DEBUG )
{
    pContext->DrawIndexed ( count, 0u, 0u );
}

void Graphics::SetProjection ( DirectX::FXMMATRIX proj ) noexcept
{
    projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection () const noexcept
{
    return projection;
}

void Graphics::EnableImgui () noexcept
{
    imguiEnabled = true;
}

void Graphics::DisableImgui () noexcept
{
    imguiEnabled = false;
}

bool Graphics::IsImguiEnabled () const noexcept
{
    return imguiEnabled;
}

void Graphics::SetCamera ( DirectX::XMMATRIX camera ) noexcept
{
    this->_camera = camera;
}

DirectX::XMMATRIX Graphics::GetCamera () const noexcept
{
    return _camera;
}

Graphics::InfoException::InfoException ( int line, const char* file, std::vector<std::string> infoMsgs ) noexcept
    : Exception ( line, file )
{
    // join all info messages with newlines into single string
    for ( const auto& m : infoMsgs )
    {
        info += m;
        info.push_back ( '\n' );
    }
    // remove final newline if exists
    if ( !info.empty () )
    {
        info.pop_back ();
    }
}

const char* Graphics::InfoException::what () const noexcept
{
    std::ostringstream oss;
    oss << GetType () << std::endl << "\n[Error Info]\n" << GetErrorInfo () << std::endl << std::endl;
    oss << GetOriginString ();
    whatBuffer = oss.str ();
    return whatBuffer.c_str ();
}

const char* Graphics::InfoException::GetType () const noexcept
{
    return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo () const noexcept
{
    return info;
}

Graphics::~Graphics ()
{
    ImGui_ImplDX11_Shutdown ();
}
