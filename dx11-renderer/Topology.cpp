#include "Topology.hpp"

Topology::Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
	:
	type(type)
{

}

void Topology::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
