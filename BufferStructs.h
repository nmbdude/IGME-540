#pragma once

#include <DirectXMath.h>
#include "Lights.h"
#include <vector>

struct VertexShaderData
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
};

struct PixelShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT2 padding;
	DirectX::XMFLOAT3 normal;
	float time;
	DirectX::XMFLOAT2 scale;
	DirectX::XMFLOAT2 offset;
	DirectX::XMFLOAT3 glowColor;
	float glowIntensity;
	DirectX::XMFLOAT3 cameraPosition;
	float padding2;
	DirectX::XMFLOAT3 ambientColor;
	float padding3;
	Light lights[5];
};
