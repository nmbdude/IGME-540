#include "Material.h"
#include "PathHelpers.h"
#include "Graphics.h"
#include <d3dcompiler.h>

using namespace DirectX;

Material::Material() : Material(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), VertexShaderPtr(), PixelShaderPtr()) {}

Material::Material(DirectX::XMFLOAT4 colorTint) : Material(colorTint, VertexShaderPtr(), PixelShaderPtr()) {}

Material::Material(const wchar_t* vertexShaderFilePath, const wchar_t* pixelShaderFilePath)
	: colorTint(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
{
	CreateVertShaderFromFile(vertexShaderFilePath);
	CreatePixelShaderFromFile(pixelShaderFilePath);
}

Material::Material(DirectX::XMFLOAT4 colorTint, const wchar_t* vertexShaderFilePath, const wchar_t* pixelShaderFilePath) :
	Material(vertexShaderFilePath, pixelShaderFilePath)
{
	this->colorTint = colorTint;
}

Material::Material(DirectX::XMFLOAT4 colorTint, VertexShaderPtr vertexShader, PixelShaderPtr pixelShader)
	: colorTint(colorTint), vertexShader(vertexShader), pixelShader(pixelShader){}

DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
VertexShaderPtr Material::GetVertexShader() { return vertexShader; }
PixelShaderPtr Material::GetPixelShader() { return pixelShader; }

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint) { this->colorTint = colorTint; }
void Material::SetVertexShader(VertexShaderPtr vertexShader) { this->vertexShader = vertexShader; }
void Material::SetPixelShader(PixelShaderPtr pixelShader) { this->pixelShader = pixelShader; }

void Material::CreateVertShaderFromFile(const wchar_t* filePath)
{
	ID3DBlob* vertexShaderBlob;
	D3DReadFileToBlob(FixPath(filePath).c_str(), &vertexShaderBlob);
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(), // Pointer to start of binary data
		vertexShaderBlob->GetBufferSize(), // How big is that data?
		0, // No classes in this shader
		vertexShader.GetAddressOf()); // ID3D11VertexShader**

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)

	D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

	// Set up the first element - a position, which is 3 float values
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
	inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
	inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

	// Set up the second element - a color, which is 4 more float values
	inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;			// 2x 32-bit floats
	inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
	inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	// Set up the second element - a color, which is 4 more float values
	inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// 3x 32-bit floats
	inputElements[2].SemanticName = "NORMAL";							// Match our vertex shader input!
	inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	// Set up the second element - a color, which is 4 more float values
	inputElements[3].Format = DXGI_FORMAT_R32_FLOAT;			// 3x 32-bit floats
	inputElements[3].SemanticName = "TIME";							// Match our vertex shader input!
	inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	// Create the input layout, verifying our description against actual shader code
	Graphics::Device->CreateInputLayout(
		inputElements,							// An array of descriptions
		4,										// How many elements in that array?
		vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
		vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
		inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer

	Graphics::Context->IASetInputLayout(inputLayout.Get());
}

void Material::CreatePixelShaderFromFile(const wchar_t* filePath)
{
	ID3DBlob* pixelShaderBlob;
	D3DReadFileToBlob(FixPath(filePath).c_str(), &pixelShaderBlob);
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(), // Pointer to start of binary data
		pixelShaderBlob->GetBufferSize(), // How big is that data?
		0, // No classes in this shader
		pixelShader.GetAddressOf()); // ID3D11PixelShader**
}