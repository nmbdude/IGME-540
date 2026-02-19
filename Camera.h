#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include "Input.h"

class Camera
{
public:
	Camera();
	Camera(float aR, DirectX::XMFLOAT3 pos);
	Camera(float aR, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot);
	Camera(float aR, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, float fov);
	Camera(float aR, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, float fov, float nearP, float farP);
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	float GetFOV();
	Transform GetTransform();

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();

	void Update(float dt);


private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float fov;
	float nearPlane;
	float farPlane;
	float movementSpeed;
	float lookSensitivity;
	float aspectRatio;
	bool isPerspective;
};

