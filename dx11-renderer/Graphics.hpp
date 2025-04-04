#pragma once
#include "MinWin.hpp"
#include <d3d11.h>
#include "Exception.hpp"
#include "DxgiInfoManager.h"
#include "MinWrl.hpp"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>
namespace Bind
{
class Bindable;
}
class Graphics
{
    friend class Bind::Bindable;

  private:
#ifndef NDEBUG
    DxgiInfoManager infoManager;
#endif
    Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
    bool imguiEnabled = true;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX _camera;

  public:
    class HrException : public Exception
    {
      private:
        HRESULT hr;
        std::string info;

      public:
        using Exception::Exception;

        HrException( int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {} ) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;
        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorString() const noexcept;
        std::string GetErrorDescription() const noexcept;
    };
    class InfoException : public Exception
    {
      public:
        InfoException( int line, const char* file, std::vector<std::string> infoMsgs ) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;
        std::string GetErrorInfo() const noexcept;

      private:
        std::string info;
    };
    class DeviceRemovedException : public HrException
    {
        using HrException::HrException;

      private:
        std::string reason;

      public:
        const char* GetType() const noexcept override;
    };

  public:
    Graphics( HWND hwnd );
    Graphics( const Graphics& )            = delete;
    Graphics& operator=( const Graphics& ) = delete;
    ~Graphics();
    void EndFrame();
    void BeginFrame( float red, float green, float blue ) noexcept;
    void DrawIndexed( UINT count ) noexcept( !IS_DEBUG );
    void SetProjection( DirectX::FXMMATRIX proj ) noexcept;
    DirectX::XMMATRIX GetProjection() const noexcept;

    void EnableImgui() noexcept;
    void DisableImgui() noexcept;
    bool IsImguiEnabled() const noexcept;
    void SetCamera( DirectX::XMMATRIX camera ) noexcept;
    DirectX::XMMATRIX GetCamera() const noexcept;
};
