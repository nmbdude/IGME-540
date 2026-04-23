#pragma once
#include <d3d11.h>
#include <memory>
#include <wrl/client.h>
#include "Actor.h"

class Material;

typedef Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShaderPtr;
typedef Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShaderPtr;
typedef Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayoutPtr;
typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRVPtr;
typedef std::shared_ptr<Material> Mat;
typedef Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
typedef std::shared_ptr<Actor> AActor;