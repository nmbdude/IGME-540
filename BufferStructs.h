#pragma once

#include <DirectXMath.h>

struct VertexShaderData
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct PixelShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT2 padding;
	DirectX::XMFLOAT3 normal;
	float time;
};
