#pragma once
#include <DirectXMath.h>
#include "TypeDefs.h"
#include <unordered_map>

class Material
{
private:
	DirectX::XMFLOAT4 colorTint;
	VertexShaderPtr vertexShader;
	PixelShaderPtr pixelShader;
	InputLayoutPtr inputLayout;

	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;


public:
	Material();
	Material(DirectX::XMFLOAT4 colorTint);
	Material(const wchar_t* vertexShaderFilePath, const wchar_t* pixelShaderFilePath);
	Material(DirectX::XMFLOAT4 colorTint, const wchar_t* vertexShaderFilePath, const wchar_t* pixelShaderFilePath);
	Material(DirectX::XMFLOAT4 colorTint, VertexShaderPtr vertexShader, PixelShaderPtr pixelShader);

	DirectX::XMFLOAT4 GetColorTint();
	VertexShaderPtr GetVertexShader();
	PixelShaderPtr GetPixelShader();

	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetVertexShader(VertexShaderPtr vertexShader);
	void SetPixelShader(PixelShaderPtr pixelShader);

	void CreateVertShaderFromFile(const wchar_t* filePath);
	void CreatePixelShaderFromFile(const wchar_t* filePath);

	void AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void BindTexturesAndSamplers();
};

