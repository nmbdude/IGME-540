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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
		Graphics::Context->VSSetConstantBuffers(
			0, // Which slot (register) to bind the buffer to?
			1, // How many are we setting right now?
			constantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
	}

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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer

		unsigned int size = sizeof(VertexShaderData);
		size = (size + 15) / 16 * 16;

		// Describe the constant buffer
		D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth = size; // Must be a multiple of 16
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());
		
		
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}

	
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//Custom Meshes
	{
		unsigned int quadIndices[] = { 0,1,2, 2,3,0 };
		Vertex quadVertices[] = {
			{ XMFLOAT3(-0.8f, +0.8f, 0.f), COLORS::RED},
			{ XMFLOAT3(-0.5f, +0.8f, 0.f), COLORS::GREEN},
			{ XMFLOAT3(-0.5f, +0.6f, 0.f), COLORS::BLUE},
			{ XMFLOAT3(-0.8f, +0.6f, 0.f), COLORS::WHITE},
		};
		Mesh quadMesh = Mesh(6, quadIndices, 4, quadVertices, "Quad");

		unsigned int spaceshipIndices[] = { 0,1,3, 1,2,3, 0,3,5, 3,4,5 };
		Vertex spaceshipVertices[] = {
			{ XMFLOAT3(+0.7f, +0.7f, 0.f), COLORS::WHITE},
			{ XMFLOAT3(+0.75f, +0.8f, 0.f), COLORS::BLACK},
			{ XMFLOAT3(+0.9f, +0.75f, 0.f), COLORS::BLACK},
			{ XMFLOAT3(+0.8f, +0.7f, 0.f), COLORS::BLACK},
			{ XMFLOAT3(+0.9f, +0.65f, 0.f), COLORS::BLACK},
			{ XMFLOAT3(+0.75f, +0.6f, 0.f), COLORS::BLACK}
		};
		Mesh spaceshipMesh = Mesh(12, spaceshipIndices, 6, spaceshipVertices, "Spaceship");

		unsigned int triangleIndices[] = { 0, 1, 2 };
		Vertex triangleVertices[] =
		{
			{ XMFLOAT3(+0.0f, +0.5f, +0.0f), COLORS::RED },
			{ XMFLOAT3(+0.5f, -0.5f, +0.0f), COLORS::BLUE},
			{ XMFLOAT3(-0.5f, -0.5f, +0.0f), COLORS::GREEN},
		};
		Mesh triangleMesh = Mesh(3, triangleIndices, 3, triangleVertices, "Triangle");

		ATriangle = Actor(std::make_shared<Mesh>(triangleMesh));
		AQuad = Actor(std::make_shared<Mesh>(quadMesh));
		ASpaceship = Actor(std::make_shared<Mesh>(spaceshipMesh));
		AOtherActor1 = AOtherActor1 = Actor(std::make_shared<Mesh>(triangleMesh));
		AOtherActor2 = AOtherActor2 = Actor(std::make_shared<Mesh>(spaceshipMesh));

		ATriangle.SetName("Triangle");
		AQuad.SetName("Quad");
		ASpaceship.SetName("Spaceship");
		AOtherActor1.SetName("Other Actor 1");
		AOtherActor2.SetName("Other Actor 2");

		AOtherActor1.GetTransform()->SetPosition(XMFLOAT3{ -0.5f, -0.5f, 0.f });
		AOtherActor2.GetTransform()->SetPosition(XMFLOAT3{ -0.55f, -0.5f, 0.f });

		actorList.push_back(std::make_shared<Actor>(ATriangle));
		actorList.push_back(std::make_shared<Actor>(AQuad));
		actorList.push_back(std::make_shared<Actor>(ASpaceship));
		actorList.push_back(std::make_shared<Actor>(AOtherActor1));
		actorList.push_back(std::make_shared<Actor>(AOtherActor2));
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
		if (ImGui::TreeNode("Mesh: Triangle"))
		{
			ImGui::Text("Triangles: %d", ATriangle.GetMesh()->GetTriangleCount());
			ImGui::Text("Vertices: %d", ATriangle.GetMesh()->GetVertexCount());
			ImGui::Text("Indices: %d", ATriangle.GetMesh()->GetIndexCount());
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mesh: Quad"))
		{
			ImGui::Text("Triangles: %d", AQuad.GetMesh()->GetTriangleCount());
			ImGui::Text("Vertices: %d", AQuad.GetMesh()->GetVertexCount());
			ImGui::Text("Indices: %d", AQuad.GetMesh()->GetIndexCount());
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mesh: Spaceship"))
		{
			ImGui::Text("Triangles: %d", ASpaceship.GetMesh()->GetTriangleCount());
			ImGui::Text("Vertices: %d", ASpaceship.GetMesh()->GetVertexCount());
			ImGui::Text("Indices: %d", ASpaceship.GetMesh()->GetIndexCount());
			ImGui::TreePop();
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
		vsData.colorTint = shaderData.colorTint;
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



