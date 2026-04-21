#pragma once
#include "TypeDefs.h"
#include "BufferStructs.h"
#include "Mesh.h"
#include "Camera.h"
#include <memory>
#include "WICTextureLoader.h"

using namespace Microsoft::WRL;

class Sky
{
private:
	ComPtr<ID3D11SamplerState> samplerState;
	static ComPtr<ID3D11ShaderResourceView> skySRV;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	ComPtr<ID3D11RasterizerState> rasterizerState;
	PixelShaderPtr pixelShader;
	VertexShaderPtr vertexShader;
	std::shared_ptr<Mesh> mesh;
	InputLayoutPtr inputLayout;

public:
	Sky(std::shared_ptr<Mesh> mesh, ComPtr<ID3D11SamplerState> samplerState);

	void SetVertexShaderFromFile(const wchar_t* filePath);
	void SetPixelShaderFromFile(const wchar_t* filePath);

	~Sky();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
	void Draw(Camera camera);
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSkySRV();
};

