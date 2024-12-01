#include "TransformCbufDoubleBoi.hpp"
using namespace Bind;
void TransformCbufDoubleBoi::UpdateAndBindImpl(Graphics& gfx)
{
	auto model = parent.GetTransformXM();
	auto view = gfx.GetCamera();
	auto proj = gfx.GetProjection();
	TransformCbuf::Transforms t;
	t.model = model;
	t.view = view;
	t.projection = proj;
	pPcBuf->Update(gfx, t);
}

TransformCbufDoubleBoi::TransformCbufDoubleBoi(Graphics& gfx, const Drawable& parent)
	:
	TransformCbuf(gfx, parent)
{
	if (!pPcBuf)
	{
		pPcBuf = std::make_unique<PixelConstantBuffer<TransformCbuf::Transforms>>(gfx, 3);
	}
}

void TransformCbufDoubleBoi::Bind(Graphics& gfx) noexcept
{
	TransformCbuf::Bind(gfx);
	UpdateAndBindImpl(gfx);
}

std::unique_ptr<PixelConstantBuffer<TransformCbuf::Transforms>> TransformCbufDoubleBoi::pPcBuf;