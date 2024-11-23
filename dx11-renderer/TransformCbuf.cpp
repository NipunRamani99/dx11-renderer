#include "TransformCbuf.hpp"
#include "ConstantBuffers.hpp"
using namespace Bind;
TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent)
	:
	parent(parent)
{
	if (!pVcbuf) {
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, 0u) ;
	}
}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
	/*const auto model = parent.GetTransformXM();
	const auto view = gfx.GetCamera();
	const Transforms tf =
	{
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(view),
		DirectX::XMMatrixTranspose(gfx.GetProjection())
	};*/
	//pVcbuf->Update(gfx, tf);
	//pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<Transforms>> TransformCbuf::pVcbuf;