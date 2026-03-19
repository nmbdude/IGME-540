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

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView = {};
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_Wood_D.jpg").c_str(),
		0,
		shaderResourceView.GetAddressOf()
	);
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Diffuse/T_Brick_D.jpg").c_str(),
		0,
		shaderResourceView.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState = {};
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	MWhite = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	MWhite->AddTextureSRV(0, shaderResourceView);
	MWhite->AddSampler(0, samplerState);
	MRed = std::make_shared<Material>(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	MRed->AddTextureSRV(1, shaderResourceView);
	MRed->AddSampler(1, samplerState);
	MGreen = std::make_shared<Material>(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	MGreen->AddTextureSRV(2, shaderResourceView);
	MGreen->AddSampler(2, samplerState);
	MBlue = std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	MBlue->AddTextureSRV(3, shaderResourceView);
	MBlue->AddSampler(3, samplerState);
	MDebugNormals = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"DebugNormalsPS.cso");
	MDebugNormals->AddTextureSRV(4, shaderResourceView);
	MDebugNormals->AddSampler(4, samplerState);
	MDebugUVs = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"DebugUVsPS.cso");
	MDebugUVs->AddTextureSRV(5, shaderResourceView);
	MDebugUVs->AddSampler(5, samplerState);
	MCustom = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), L"VertexShader.cso", L"CustomPS.cso");
	MCustom->AddTextureSRV(6, shaderResourceView);
	MCustom->AddSampler(6, samplerState);
	

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

	CreateGeometry();
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
	{
		std::shared_ptr<Mesh> MSphere = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str());
		std::shared_ptr<Mesh> MQuad = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str());
		std::shared_ptr<Mesh> MCylinder = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str());
		std::shared_ptr<Mesh> MHelix = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str());
		std::shared_ptr<Mesh> MCube = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str());
		std::shared_ptr<Mesh> MTorus = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str());

		meshList.push_back(MSphere);
		meshList.push_back(MQuad);
		meshList.push_back(MCylinder);
		meshList.push_back(MHelix);
		meshList.push_back(MCube);
		meshList.push_back(MTorus);
	}

	CreateRowOfGeometry(MWhite, 3.f, -7.f, 5.f);
	CreateRowOfGeometry(MRed, 0.f, -7.f, 5.f);
	CreateRowOfGeometry(MCustom, -3.f, -7.f, 5.f);
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
	Actor cube = Actor(meshList[4], material, "Cube##" + std::to_string(randomID));
	cube.GetTransform()->SetPosition(XMFLOAT3{ 0.f + xOffset, y, 0.f + zOffset });
	cube.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(cube));

	Actor cylinder = Actor(meshList[2], material, "Cylinder##" + std::to_string(randomID));
	cylinder.GetTransform()->SetPosition(XMFLOAT3{ 3.f + xOffset, y, 0.f + zOffset });
	cylinder.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(cylinder));

	Actor helix = Actor(meshList[3], material, "Helix##" + std::to_string(randomID));
	helix.GetTransform()->SetPosition(XMFLOAT3{ 6.f + xOffset, y, 0.f + zOffset });
	helix.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(helix));

	Actor sphere = Actor(meshList[0], material, "Sphere##" + std::to_string(randomID));
	sphere.GetTransform()->SetPosition(XMFLOAT3{ 9.f + xOffset, y, 0.f + zOffset });
	sphere.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(sphere));

	Actor torus = Actor(meshList[5], material, "Torus##" + std::to_string(randomID));
	torus.GetTransform()->SetPosition(XMFLOAT3{ 12.f + xOffset, y, 0.f + zOffset });
	torus.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(torus));

	Actor quad = Actor(meshList[1], material, "Quad##" + std::to_string(randomID));
	quad.GetTransform()->SetPosition(XMFLOAT3{ 15.f + xOffset, y, 0.f + zOffset });
	quad.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(quad));

	Actor quad2 = Actor(meshList[1], material, "Quad2##" + std::to_string(randomID));
	quad2.GetTransform()->SetPosition(XMFLOAT3{ 18.f + xOffset, y, 0.f + zOffset });
	quad2.GetTransform()->SetRotation(0.f, XMConvertToRadians(90.f), 0.f);
	actorList.push_back(std::make_shared<Actor>(quad2));
}


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
				count++;
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Meshes"))
	{
		for (int i = 0; i < meshList.size(); i++)
		{
			if (ImGui::TreeNode("Mesh: Sphere"))
			{
				ImGui::Text("Triangles: %d", ASphere.GetMesh()->GetTriangleCount());
				ImGui::Text("Vertices: %d", ASphere.GetMesh()->GetVertexCount());
				ImGui::Text("Indices: %d", ASphere.GetMesh()->GetIndexCount());
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
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
		//ImGui::ColorEdit4("Tint Color", (float*)&shaderData.colorTint);
		ImGui::TreePop();
	}
	if (ImGui::Button("Toggle Demo Window"))
	{
		demoVisible = !demoVisible;
	}

	ImGui::End();
#pragma endregion

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
	

	for(std::shared_ptr<Actor> actor : actorList)
	{
		VertexShaderData vsData = {};
		vsData.matrix = actor->GetTransform()->GetWorldMatrix();
		vsData.view = activeCamera->GetViewMatrix();
		vsData.projection = activeCamera->GetProjectionMatrix();

		Graphics::FillAndBindNextConstantBuffer(
			&vsData,
			sizeof(VertexShaderData),
			D3D11_VERTEX_SHADER,
			0);

		PixelShaderData psData = {};
		psData.colorTint = actor->GetMaterial()->GetColorTint();
		psData.time = totalTime;

		Graphics::FillAndBindNextConstantBuffer(
			&psData,
			sizeof(PixelShaderData),
			D3D11_PIXEL_SHADER,
			0);
		
		actor->GetMaterial()->BindTexturesAndSamplers();
		actor->Draw();
	}

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
	}
}



