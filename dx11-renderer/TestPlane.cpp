#include "TestPlane.hpp"
#include "imgui\imgui.h"
TestPlane::TestPlane(Graphics& gfx)
{
	auto model = Plane::Make();
	AddBind(Bind::VertexBuffer::Resolve(gfx, "TestPlane", model.vertices));
	AddBind(Bind::IndexBuffer::Resolve(gfx, "TestPlane", model.indices));
	AddBind(Bind::Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	auto vs = Bind::VertexShader::Resolve(gfx, "./PhongVSTexturedTBN.cso");
	auto pvsbc = vs->GetBytecode();
	AddBind(vs);
	AddBind(Bind::PixelShader::Resolve(gfx, "./PhongPSTexturedTBNSpecular.cso"));
	AddBind(Bind::InputLayout::Resolve(gfx, model.vertices.GetVertexLayout(), pvsbc));
	AddBind(Bind::Texture::Resolve(gfx, "./models/brick_wall/brick_wall_diffuse.jpg", 1u));
	AddBind(Bind::Texture::Resolve(gfx, "./models/brick_wall/brick_wall_normal.jpg", 2u));
	AddBind(Bind::Sampler::Resolve(gfx, 1u));

	struct ObjectData 
	{
		alignas(16) DirectX::XMFLOAT3 material;
		float specularIntensity = 0.60f;
		float specularPower = 30.0f;
		BOOL normalMapEnabled = FALSE;
		float padding[1];
		static std::string GetId()
		{
			return "ObjectDataTestPlane";
		}
	} objectData;
	objectData.material = { 1.0f, 0.2f, 0.1f };
	AddBind(Bind::PixelConstantBuffer<ObjectData>::Resolve(gfx, objectData, 1));
	AddBind(std::make_shared<Bind::TransformCbufDoubleBoi>(gfx, *this));
}

DirectX::XMMATRIX TestPlane::GetTransformXM() const noexcept
{
	return   DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(_roll), DirectX::XMConvertToRadians(_pitch),DirectX::XMConvertToRadians(_yaw)) *
		DirectX::XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
}

void TestPlane::SpawnControl() noexcept
{
	ImGui::Begin("Test Plane");
	ImGui::Text("Position");
	ImGui::SameLine();
	ImGui::InputFloat3("##PositionInput", (float*)&_pos);
	ImGui::Text("Scale");
	ImGui::SameLine();
	ImGui::InputFloat3("##ScaleInput", (float*)&_scale);
	ImGui::Text("Pitch");
	ImGui::SameLine();
	ImGui::InputFloat("##PitchInput", &_pitch);
	ImGui::Text("Yaw");
	ImGui::SameLine();
	ImGui::InputFloat("##YawInput", &_yaw);
	ImGui::Text("Roll");
	ImGui::SameLine();
	ImGui::InputFloat("##RollInput", &_roll);
	ImGui::End();
}
