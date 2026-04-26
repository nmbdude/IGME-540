#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Material.h"
#include "Graphics.h"
#include "BufferStructs.h"
#include "Mesh.h"
#include "TypeDefs.h"
#include "Actor.h"
#include "Transform.h"
#include <memory>
#include "Camera.h"
#include <vector>
#include "Lights.h"
#include "Sky.h"
#include "Gizmo.h"

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();
private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();
	void NewFrame(float deltaTime);
	void CreateRowOfGeometry(std::shared_ptr<Material> material, float y, float xOffset, float zOffset);
	void CreateShadowMap();
	void UpdateLightView(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 position);
	void UpdateLightView(DirectX::XMFLOAT3 direction);
	void ShadowMapRender();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pixelConstantBuffer;

	std::vector<std::shared_ptr<Camera>> cameras;

	std::shared_ptr<Camera> activeCamera;
	int activeCameraIndex;

	std::shared_ptr<Camera> camera1;
	std::shared_ptr<Camera> camera2;
	std::shared_ptr<Camera> camera3;

	VertexShaderData shaderData;
	std::vector<std::shared_ptr<Actor>> actorList;
	std::vector<std::shared_ptr<Mesh>> meshList;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState = {};

	DirectX::XMFLOAT3 ambientColor = { 0.276f, 0.276f, 0.569f };
	std::vector<Light> lights;

	std::shared_ptr<Sky> sky;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;

	Actor selectedActor;
	Gizmo gizmo;

	//New Actors
	Actor ASphere;
	Actor AQuad;
	Actor AHelix;
	Actor ACylinder;
	Actor ACube;

	// Materials
	Mat M_Wood;
	Mat M_GlowingBricks;
	Mat M_Red;
	Mat M_Green;
	Mat M_Blue;
	Mat M_DebugNormals;
	Mat M_DebugUVs;
	Mat M_Custom;
	Mat M_Stone;
	Mat M_Bronze;
	Mat M_Grass;

	// User controls
	float backgroundColor[4];
	bool demoVisible;
	bool rainbowMode;
	float rainbowSpeed;
	DirectX::XMFLOAT3 glowColor = {1.0f, 1.0f, 0.0f};
	float glowIntensity = 1.0f;

	// Shadows
	InputLayoutPtr inputLayout;
	VertexShaderPtr shadowVS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	float lightProjectionSize = 20.0f;
	float shadowMapResolution = 2048;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ppVS;
	// Resources that are tied to a particular post process
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling
	PixelShaderPtr bbPS;
	float blurRadius = 5.f;
};

