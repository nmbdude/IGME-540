#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"
#include <DirectXMath.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "WICTextureLoader.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	srand((unsigned int)time(0));

	Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_Wood_D.jpg").c_str(),
		0,
		woodSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> stoneSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_Stone_D.jpg").c_str(),
		0,
		stoneSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> stoneNormalsSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Normal/T_Stone_N.png").c_str(),
		0,
		stoneNormalsSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSpecularSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Specular/T_Wood_S.png").c_str(),
		0,
		woodSpecularSRV.GetAddressOf()
	);
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_Brick_D.jpg").c_str(),
		0,
		brickSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glowSRV = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_BrickMask_D.png").c_str(),
		0,
		glowSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> T_Bronze_A = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(),
		0,
		T_Bronze_A.GetAddressOf()
	);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> T_Bronze_N = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(),
		0,
		T_Bronze_N.GetAddressOf()
	);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> T_Bronze_R = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(),
		0,
		T_Bronze_R.GetAddressOf()
	);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> T_Bronze_M = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(),
		0,
		T_Bronze_M.GetAddressOf()
	);

	shadowDSV = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>{};
	CreateShadowMap();

	
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	{
		std::shared_ptr<Mesh> SM_Sphere = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str());
		std::shared_ptr<Mesh> SM_Quad = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str());
		std::shared_ptr<Mesh> SM_Cylinder = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str());
		std::shared_ptr<Mesh> SM_Helix = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str());
		std::shared_ptr<Mesh> SM_Cube = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str());
		std::shared_ptr<Mesh> SM_Torus = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str());
		
		meshList.push_back(SM_Sphere);
		meshList.push_back(SM_Quad);
		meshList.push_back(SM_Cylinder);
		meshList.push_back(SM_Helix);
		meshList.push_back(SM_Cube);
		meshList.push_back(SM_Torus);
	}
	
	sky = std::make_shared<Sky>(meshList[4], samplerState);
	skySRV = sky->CreateCubemap(
		FixPath(L"../../Assets/Textures/Skybox/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/back.png").c_str()
	);
	sky->SetVertexShaderFromFile(L"SkyVS.cso");
	sky->SetPixelShaderFromFile(L"SkyPS.cso");


	M_Wood = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	M_Wood->AddTextureSRV(0, woodSRV);
	M_Wood->AddTextureSRV(1, woodSpecularSRV);
	M_Wood->AddSampler(0, samplerState);

	M_Stone = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"NormalMapVS.cso", L"NormalMapPS.cso");
	M_Stone->AddTextureSRV(0, stoneSRV);
	M_Stone->AddTextureSRV(1, stoneNormalsSRV);
	M_Stone->AddSampler(0, samplerState);

	M_GlowingBricks = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"GlowingBricksPS.cso");
	M_GlowingBricks->AddTextureSRV(0, brickSRV);
	M_GlowingBricks->AddTextureSRV(1, glowSRV);
	M_GlowingBricks->AddSampler(0, samplerState);

	M_Red = std::make_shared<Material>(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	M_Red->AddTextureSRV(0, brickSRV);
	M_Red->AddTextureSRV(1, woodSpecularSRV);
	M_Red->AddSampler(0, samplerState);

	M_Green = std::make_shared<Material>(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	M_Green->AddTextureSRV(0, woodSRV);
	M_Green->AddTextureSRV(1, woodSpecularSRV);
	M_Green->AddSampler(0, samplerState);

	M_Blue = std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	M_Blue->AddTextureSRV(0, brickSRV);
	M_Blue->AddTextureSRV(1, woodSpecularSRV);
	M_Blue->AddSampler(0, samplerState);

	M_DebugNormals = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"DebugNormalsPS.cso");
	M_DebugNormals->AddTextureSRV(0, woodSRV);
	M_DebugNormals->AddSampler(0, samplerState);

	M_DebugUVs = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"DebugUVsPS.cso");
	M_DebugUVs->AddTextureSRV(0, brickSRV);
	M_DebugUVs->AddSampler(0, samplerState);

	M_Custom = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"CustomPS.cso");
	M_Custom->AddTextureSRV(0, woodSRV);
	M_Custom->AddSampler(0, samplerState);

	M_Bronze = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"NormalMapVS.cso", L"PBRNormalMapPS.cso");
	M_Bronze->AddTextureSRV(0, T_Bronze_A);
	M_Bronze->AddTextureSRV(1, T_Bronze_N);
	M_Bronze->AddTextureSRV(2, T_Bronze_R);
	M_Bronze->AddTextureSRV(3, T_Bronze_M);
	M_Bronze->AddSampler(0, samplerState);

	CreateGeometry();

	

	// Lights -------------------------------------------------------------
	
	// Yellow Directional Light
	Light directionalLight1 = {};
	directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.Direction = XMFLOAT3(0.4f, -1.0f, 1.2f);
	directionalLight1.Color = XMFLOAT3(1.0, 1.0, 1.0);
	directionalLight1.intensity = 5.0f;
	lights.push_back(directionalLight1);
	UpdateLightView(directionalLight1.Direction);

	// Red Directional Light
	Light directionalLight2 = {};
	directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight2.Color = XMFLOAT3(1.0, 0.0, 0.0);
	directionalLight2.intensity = 5.0f;
	lights.push_back(directionalLight2);

	// Blue Directional Light
	Light directionalLight3 = {};
	directionalLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.Direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.Color = XMFLOAT3(0.0, 0.0, 1.0);
	directionalLight3.intensity = 5.0f;
	lights.push_back(directionalLight3);

	Light spotLight1 = {};
	spotLight1.Type = LIGHT_TYPE_SPOT;
	spotLight1.Position = XMFLOAT3(11.0f, 5.0f, 5.0f);
	spotLight1.Direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	spotLight1.Color = XMFLOAT3(1.0, 1.0, 1.0);
	spotLight1.Range = 10.0f;
	spotLight1.SpotInnerAngle = 3.0f;
	spotLight1.SpotOuterAngle = 20.0f;
	spotLight1.intensity = 5.0f;
	lights.push_back(spotLight1);

	Light pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Position = XMFLOAT3(5.0f, 6.0f, 4.0f);
	pointLight1.Color = XMFLOAT3(0.0, 1.0, 0.0);
	pointLight1.intensity = 5.0f;
	pointLight1.Range = 10.0f;

	lights.push_back(pointLight1);


	// --------------------------------------------------------------------

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();


	camera1 = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, -1.0f));
	camera2 = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, XMConvertToRadians(45.f), 0.0f), 45.0f);
	camera3 = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, XMConvertToRadians(-45.f), 0.0f), 70.0f);

	cameras.push_back(camera1);
	cameras.push_back(camera2);
	cameras.push_back(camera3);

	activeCamera = camera1;
	activeCameraIndex = 0;

	backgroundColor[0] = 0.4f;
	backgroundColor[1] = 0.6f;
	backgroundColor[2] = 0.75f;
	backgroundColor[3] = 1.0f;
	demoVisible = false;
	rainbowMode = false;
	rainbowSpeed = 1.0f;
	//shaderData.colorTint = XMFLOAT4{1.f,1.f,1.f,1.f};
	inputLayout = InputLayoutPtr{};
	ID3DBlob* vertexShaderBlob;
	D3DReadFileToBlob(FixPath(L"ShadowVS.cso").c_str(), &vertexShaderBlob);
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(), // Pointer to start of binary data
		vertexShaderBlob->GetBufferSize(), // How big is that data?
		0, // No classes in this shader
		shadowVS.GetAddressOf()); // ID3D11VertexShader**
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//Custom Meshes
	CreateRowOfGeometry(M_Bronze, 3.f, -7.f, 5.f);
	std::shared_ptr<Actor> floor = std::make_shared<Actor>(meshList[1], M_Stone, "Floor");
	floor->GetTransform()->SetPosition(0.f, -1.f, 0.f);
	floor->GetTransform()->SetScale(20.f, 1.f, 20.f);
	actorList.push_back(floor);

	
}

void Game::NewFrame(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	if(demoVisible)
	{
		ImGui::ShowDemoWindow();
	}
}

void Game::CreateRowOfGeometry(std::shared_ptr<Material> material, float y, float xOffset, float zOffset)
{
	int randomID = rand() % 1000;
	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor cube = Actor(meshList[4], mat, "Cube##" + std::to_string(randomID));
		cube.GetTransform()->SetPosition(XMFLOAT3{ 0.f + xOffset, y, 0.f + zOffset });
		cube.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(cube));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor cylinder = Actor(meshList[2], mat, "Cylinder##" + std::to_string(randomID));
		cylinder.GetTransform()->SetPosition(XMFLOAT3{ 3.f + xOffset, y, 0.f + zOffset });
		cylinder.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(cylinder));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor helix = Actor(meshList[3], mat, "Helix##" + std::to_string(randomID));
		helix.GetTransform()->SetPosition(XMFLOAT3{ 6.f + xOffset, y, 0.f + zOffset });
		helix.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(helix));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor sphere = Actor(meshList[0], mat, "Sphere##" + std::to_string(randomID));
		sphere.GetTransform()->SetPosition(XMFLOAT3{ 9.f + xOffset, y, 0.f + zOffset });
		sphere.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(sphere));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor torus = Actor(meshList[5], mat, "Torus##" + std::to_string(randomID));
		torus.GetTransform()->SetPosition(XMFLOAT3{ 12.f + xOffset, y, 0.f + zOffset });
	 torus.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(torus));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor quad = Actor(meshList[1], mat, "Quad##" + std::to_string(randomID));
		quad.GetTransform()->SetPosition(XMFLOAT3{ 15.f + xOffset, y, 0.f + zOffset });
		quad.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(quad));
	}

	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(*material);
		Actor quad2 = Actor(meshList[1], mat, "Quad2##" + std::to_string(randomID));
		quad2.GetTransform()->SetPosition(XMFLOAT3{ 18.f + xOffset, y, 0.f + zOffset });
		quad2.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
		actorList.push_back(std::make_shared<Actor>(quad2));
	}
}

// Shadow Methods ------------------------------------------------------------

void Game::CreateShadowMap()
{
	D3D11_TEXTURE2D_DESC shadowMapDesc = {};
	shadowMapDesc.Width = shadowMapResolution;
	shadowMapDesc.Height = shadowMapResolution;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.MiscFlags = 0;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	Texture shadowMapTexture;
	Graphics::Device->CreateTexture2D(&shadowMapDesc, 0, shadowMapTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowMapTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());
	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowMapTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRasterizerDesc = {};
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_BACK;
	shadowRasterizerDesc.DepthClipEnable = false;
	shadowRasterizerDesc.DepthBias = 1000;
	shadowRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	Graphics::Device->CreateRasterizerState(&shadowRasterizerDesc, shadowRasterizer.GetAddressOf());

	D3D11_SAMPLER_DESC shadowSamplerDesc = {};
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	Graphics::Device->CreateSamplerState(&shadowSamplerDesc, shadowSampler.GetAddressOf());
}

void Game::UpdateLightView(XMFLOAT3 direction, XMFLOAT3 position)
{
	XMVECTOR dirVector = XMLoadFloat3(&direction);
	XMVECTOR downVector = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
	if (fabsf(XMVectorGetX(XMVector3Dot(dirVector, worldUp))) > 0.99f)
		worldUp = XMVectorSet(0, 0, 1, 0);

	XMMATRIX lightView = XMMatrixLookToLH(
		XMVectorAdd(XMVectorScale(-dirVector, 20.0f), XMLoadFloat3(&position)),
		dirVector,
		worldUp
	);
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f
	);

	XMStoreFloat4x4(&lightViewMatrix, lightView);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

void Game::UpdateLightView(XMFLOAT3 direction)
{
	XMVECTOR dirVector = XMLoadFloat3(&direction);
	XMVECTOR downVector = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
	if (fabsf(XMVectorGetX(XMVector3Dot(dirVector, worldUp))) > 0.99f)
		worldUp = XMVectorSet(0, 0, 1, 0);

	XMMATRIX lightView = XMMatrixLookToLH(
		XMVectorScale(-dirVector, 20.0f),
		dirVector,
		worldUp
	);
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f
	);

	XMStoreFloat4x4(&lightViewMatrix, lightView);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

struct ShadowVSData
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
};

void Game::ShadowMapRender()
{
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context = Graphics::Context;
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	context->VSSetShader(shadowVS.Get(), 0, 0);

	context->RSSetState(shadowRasterizer.Get());

	ShadowVSData vsData = {};
	vsData.view = lightViewMatrix;
	vsData.proj = lightProjectionMatrix;

	for (auto& a : actorList)
	{
		vsData.world = a->GetTransform()->GetWorldMatrix();
		Graphics::FillAndBindNextConstantBuffer(
			&vsData,
			sizeof(ShadowVSData),
			D3D11_VERTEX_SHADER,
			0);
		a->GetMesh()->Draw();
	}

	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get()
	);

	context->RSSetState(0);
}

// -------------------------------------------------------------------------

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (std::shared_ptr<Camera> camera : cameras)
	{
		if (camera) { camera->UpdateProjectionMatrix(Window::AspectRatio()); }
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	NewFrame(deltaTime);
	for (std::shared_ptr<Actor> actor : actorList)
	{
		//actor->GetTransform()->SetPosition(sinf(totalTime) * 0.5f, actor->GetTransform()->GetPosition().y, actor->GetTransform()->GetPosition().z);
		//actor->GetTransform()->Rotate(0.0f, 0.0f, deltaTime * 0.5f);
		//actor->GetTransform()->SetScale(1.0f + 0.1f * sinf(totalTime * 2), 1.0f + 0.5f * sinf(totalTime), 1.0f);
	}
		
#pragma region UI
	// Custom windows
	ImGui::Begin("Details");
	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));
	if(ImGui::TreeNode("App Details"))
	{
		ImGui::Text("Frame Rate: %.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Text("Window Size: %d x %d", Window::Width(), Window::Height());
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Cameras"))
	{
		ImGui::Text("Camera Count: %d", cameras.size());
		int count = 1;
		if (ImGui::Combo("Camera Quick Switch", &activeCameraIndex, "Camera 1\0Camera 2\0Camera 3\0")) { activeCamera = cameras[activeCameraIndex]; }
		for (std::shared_ptr<Camera> camera : cameras)
		{
			std::string label = "Camera " + std::to_string(count);
			if (ImGui::TreeNode(label.c_str()))
			{
				if (ImGui::Button("Set Active"))
				{
					activeCamera = camera;
					activeCameraIndex = count - 1;
				}
				XMFLOAT3 position = camera->GetTransform().GetPosition();
				XMFLOAT3 rotation = camera->GetTransform().GetPitchYawRoll();
				ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
				ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", XMConvertToDegrees(rotation.x), XMConvertToDegrees(rotation.y), XMConvertToDegrees(rotation.z));
				ImGui::Text("FOV: %.1f", camera->GetFOV());
				ImGui::TreePop();
			}
			count++;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Actors"))
	{
		ImGui::Text("Actor Count: %d", actorList.size());
		// Use a counter to create unique labels for ImGui widgets in case they have the same name
		int count = 0;
		for (std::shared_ptr<Actor> actor : actorList)
		{
			std::string label = actor->GetName().c_str() + std::to_string(count);
			if (ImGui::TreeNode(label.c_str()))
			{
				if(ImGui::TreeNode("Transform"))
				{
					XMFLOAT3 position = actor->GetTransform()->GetPosition();
					XMFLOAT3 rotation = actor->GetTransform()->GetPitchYawRoll();
					XMFLOAT3 scale = actor->GetTransform()->GetScale();
					std::string posLabel = "Position##" + actor->GetName() + std::to_string(count);
					std::string rotLabel = "Rotation##" + actor->GetName() + std::to_string(count);
					std::string scaleLabel = "Scale##" + actor->GetName() + std::to_string(count);

					ImGui::Text("Mesh: %s", actor->GetMesh()->GetName().c_str());

					if (ImGui::DragFloat3(posLabel.c_str(), (float*)&position, 0.01f))
					{
						actor->GetTransform()->SetPosition(position);
					}
					if (ImGui::DragFloat3(rotLabel.c_str(), (float*)&rotation, 0.01f))
					{
						actor->GetTransform()->SetRotation(rotation);
					}
					if (ImGui::DragFloat3(scaleLabel.c_str(), (float*)&scale, 0.01f))
					{
						actor->GetTransform()->SetScale(scale);
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Material"))
				{
					XMFLOAT4 colorTint = actor->GetMaterial()->GetColorTint();
					XMFLOAT2 textureScale = actor->GetMaterial()->GetTextureScale();
					XMFLOAT2 textureOffset = actor->GetMaterial()->GetTextureOffset();
					std::string colorLabel = "Color Tint##" + actor->GetName() + std::to_string(count);
					if (ImGui::ColorEdit4(colorLabel.c_str(), (float*)&colorTint))
					{
						actor->GetMaterial()->SetColorTint(colorTint);
					}
					if (ImGui::DragFloat2("Texture Scale", (float*)&textureScale, 0.01f))
					{
						actor->GetMaterial()->SetTextureScale(textureScale);
					}
					if (ImGui::DragFloat2("Texture Offset", (float*)&textureOffset, 0.01f))
					{
						actor->GetMaterial()->SetOffset(textureOffset);
					}
					for (auto& [slot, srv] : actor->GetMaterial()->GetAllTextureSRVs())
					{
						ImGui::Image(static_cast<void*>(srv.Get()), ImVec2(256, 256));
					}
					ImGui::TreePop();
				}
				count++;
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lights"))
	{
		ImGui::ColorEdit3("Ambient Color", (float*)&ambientColor); 
;		ImGui::Text("Light Count: %d", lights.size());
		int count = 1;
		for (Light& light : lights)
		{
			std::string label = "Light " + std::to_string(count);
			if (ImGui::TreeNode(label.c_str()))
			{
				std::string typeLabel = "Type##" + std::to_string(count);
				std::string intensityLabel = "Intensity##" + std::to_string(count);
				ImGui::Text("Type: %s", light.Type == LIGHT_TYPE_DIRECTIONAL ? "Directional" : light.Type == LIGHT_TYPE_POINT ? "Point" : "Spot");
				ImGui::ColorEdit3("Color", (float*)&light.Color);
				if (light.Type == LIGHT_TYPE_DIRECTIONAL)
				{
					if (ImGui::DragFloat3("Direction", (float*)&light.Direction, 0.01f))
					{
						UpdateLightView(light.Direction);
					}
				}
				else if (light.Type == LIGHT_TYPE_POINT)
				{
					if (ImGui::DragFloat3("Position", (float*)&light.Position, 0.01f))
					{
						//UpdateLightView
					}
					ImGui::DragFloat("Range", (float*)&light.Range, 0.01f);
				}
				else if (light.Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat3("Position", (float*)&light.Position, 0.01f);
					ImGui::DragFloat3("Direction", (float*)&light.Direction, 0.01f);
					ImGui::DragFloat("Inner Angle", &light.SpotInnerAngle, 0.1f, 0.f, 180.f);
					ImGui::DragFloat("Outer Angle", &light.SpotOuterAngle, 0.1f, 0.f, 180.f);
				}
				ImGui::DragFloat(intensityLabel.c_str(), &light.intensity, 0.01f, 0.f, 100.f);
				ImGui::TreePop();
			}
			count++;
		}
		ImGui::TreePop();
	}

	//if (ImGui::TreeNode("Meshes"))
	//{
	//	for (int i = 0; i < meshList.size(); i++)
	//	{
	//		if (ImGui::TreeNode("Mesh: Sphere"))
	//		{
	//			ImGui::Text("Triangles: %d", ASphere.GetMesh()->GetTriangleCount());
	//			ImGui::Text("Vertices: %d", ASphere.GetMesh()->GetVertexCount());
	//			ImGui::Text("Indices: %d", ASphere.GetMesh()->GetIndexCount());
	//			ImGui::TreePop();
	//		}
	//	}
	//	ImGui::TreePop();
	//}
	if(ImGui::TreeNode("Customization"))
	{
		ImGui::Checkbox("Rainbow Mode", &rainbowMode);
		if (rainbowMode) 
		{
			backgroundColor[0] += deltaTime/2 * rainbowSpeed;
			backgroundColor[1] += deltaTime/3 * rainbowSpeed;
			backgroundColor[2] += deltaTime/4 * rainbowSpeed;
			if (backgroundColor[0] > 1.0f) backgroundColor[0] = 0.0f;
			if (backgroundColor[1] > 1.0f) backgroundColor[1] = 0.0f;
			if (backgroundColor[2] > 1.0f) backgroundColor[2] = 0.0f;
			ImGui::SliderFloat("Rainbow Speed", &rainbowSpeed, 0.1f, 5.0f);
		}
		else
		{
			ImGui::ColorEdit4("Background Color", backgroundColor);
		}
		ImGui::ColorEdit3("Glow Color", (float*)&glowColor);
		ImGui::DragFloat("Glow Intensity", &glowIntensity, 0.01f, 0.f, 5.f);
		//ImGui::ColorEdit4("Tint Color", (float*)&shaderData.colorTint);
		ImGui::TreePop();
	}
	if (ImGui::Button("Toggle Demo Window"))
	{
		demoVisible = !demoVisible;
	}

	ImGui::End();
#pragma endregion

	if (Input::MouseLeftPress())
	{

	}

	activeCamera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	ShadowMapRender();

	Graphics::Context->PSSetShaderResources(4, 1, shadowSRV.GetAddressOf());
	Graphics::Context->PSSetSamplers(1, 1, shadowSampler.GetAddressOf());

	for(std::shared_ptr<Actor> actor : actorList)
	{
		VertexShaderData vsData = {};
		vsData.matrix = actor->GetTransform()->GetWorldMatrix();
		vsData.view = activeCamera->GetViewMatrix();
		vsData.projection = activeCamera->GetProjectionMatrix();
		vsData.worldInverseTranspose = actor->GetTransform()->GetWorldInverseTransposeMatrix();
		vsData.lightView = lightViewMatrix;
		vsData.lightProjection = lightProjectionMatrix;

		Graphics::FillAndBindNextConstantBuffer(
			&vsData,
			sizeof(VertexShaderData),
			D3D11_VERTEX_SHADER,
			0);

		PixelShaderData psData = {};
		psData.colorTint = actor->GetMaterial()->GetColorTint();
		psData.time = totalTime;
		psData.scale = actor->GetMaterial()->GetTextureScale();
		psData.offset = actor->GetMaterial()->GetTextureOffset();
		psData.glowColor = glowColor;
		psData.glowIntensity = glowIntensity;
		psData.cameraPosition = activeCamera->GetTransform().GetPosition();
		psData.ambientColor = ambientColor;
		memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());
		psData.lightCount = (int)lights.size();

		Graphics::FillAndBindNextConstantBuffer(
			&psData,
			sizeof(PixelShaderData),
			D3D11_PIXEL_SHADER,
			0);
		
		actor->GetMaterial()->BindTexturesAndSamplers();
		actor->Draw();
	}

	sky->Draw(*activeCamera.get());

	// ImGui Render
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());

		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}
}