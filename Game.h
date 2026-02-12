#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "BufferStructs.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "Transform.h"
#include <memory>
#include <vector>

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
	void LoadShaders();
	void CreateGeometry();
	void NewFrame(float deltaTime);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// User controls
	float backgroundColor[4];
	bool demoVisible;
	bool rainbowMode;
	float rainbowSpeed;

	VertexShaderData vsData;

	VertexShaderData shaderData;

	Transform transform;

	std::vector<std::shared_ptr<Mesh>> meshList;

	std::shared_ptr<Mesh> triangleMesh;
	std::shared_ptr<Mesh> quadMesh;
	std::shared_ptr<Mesh> spaceshipMesh;
};

