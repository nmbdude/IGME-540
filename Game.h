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

	//New Actors
	Actor ASphere;
	Actor AQuad;
	Actor AHelix;
	Actor ACylinder;
	Actor ACube;

	std::shared_ptr<Material> MRed;
	std::shared_ptr<Material> MGreen;
	std::shared_ptr<Material> MBlue;
	std::shared_ptr<Material> MDebugNormals;
	std::shared_ptr<Material> MDebugUVs;
	std::shared_ptr<Material> MCustom;

	// User controls
	float backgroundColor[4];
	bool demoVisible;
	bool rainbowMode;
	float rainbowSpeed;
};

