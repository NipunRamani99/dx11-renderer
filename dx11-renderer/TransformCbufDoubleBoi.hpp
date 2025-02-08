#pragma once
#include <DirectXMath.h>
#include <memory>
#include "TransformCbuf.hpp"

namespace Bind
{
class TransformCbufDoubleBoi : public TransformCbuf
{
  private:
    static std::unique_ptr<PixelConstantBuffer<TransformCbuf::Transforms>> pPcBuf;
    void UpdateAndBindImpl( Graphics& gfx );

  public:
    TransformCbufDoubleBoi( Graphics& gfx, const Drawable& parent );
    void Bind( Graphics& gfx ) noexcept override;
};
} // namespace Bind
