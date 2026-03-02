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
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());
	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
	Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	material1 = std::make_shared<Material>(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	material2 = std::make_shared<Material>(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");
	material3 = std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), L"VertexShader.cso", L"PixelShader.cso");

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
	shaderData.colorTint = XMFLOAT4{1.f,1.f,1.f,1.f};

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

		meshList.push_back(MSphere);
		meshList.push_back(MQuad);
		meshList.push_back(MCylinder);
		meshList.push_back(MHelix);
		meshList.push_back(MCube);

		ASphere = Actor(MSphere, material1);
		AQuad = Actor(MQuad, material2);
		ACylinder = Actor(MCylinder, material3);
		AHelix = Actor(MHelix, material1);
		ACube = Actor(MCube, material3);
		ASphere.SetName("Sphere");
		AQuad.SetName("Quad");
		ACylinder.SetName("Cylinder");
		AHelix.SetName("Helix");
		ACube.SetName("Cube");

		AHelix.GetTransform()->SetPosition(XMFLOAT3{ -0.5f, -0.5f, 0.f });
		ACube.GetTransform()->SetPosition(XMFLOAT3{ -0.55f, -0.5f, 0.f });
		actorList.push_back(std::make_shared<Actor>(ASphere));
		actorList.push_back(std::make_shared<Actor>(AQuad));
		actorList.push_back(std::make_shared<Actor>(ACylinder));
		actorList.push_back(std::make_shared<Actor>(AHelix));
		actorList.push_back(std::make_shared<Actor>(ACube));
	}
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
		actor->GetTransform()->SetPosition(sinf(totalTime) * 0.5f, actor->GetTransform()->GetPosition().y, actor->GetTransform()->GetPosition().z);
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
			if (ImGui::TreeNode(actor->GetName().c_str()))
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
		ImGui::ColorEdit4("Tint Color", (float*)&shaderData.colorTint);
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
		vsData.colorTint = actor->GetMaterial()->GetColorTint();
		vsData.matrix = actor->GetTransform()->GetWorldMatrix();
		vsData.view = activeCamera->GetViewMatrix();
		vsData.projection = activeCamera->GetProjectionMatrix();

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		Graphics::Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
		Graphics::Context->Unmap(constantBuffer.Get(), 0);
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



