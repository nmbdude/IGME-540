#define _USE_MATH_DEFINES
#include "Camera.h"
#include <cmath>
#include <iostream>
using namespace DirectX;

Camera::Camera() : Camera(1.7f, XMFLOAT3(0.0f, 0.0f, 0.0f)) {}
Camera::Camera(float aR, XMFLOAT3 pos) : Camera(aR, pos, XMFLOAT3(0.0f, 0.0f, 0.0f)) {}
Camera::Camera(float aR, XMFLOAT3 pos, XMFLOAT3 rot) : Camera(aR, pos, rot, 90.0f, 0.1f, 200.0f) {}
Camera::Camera(float aR, XMFLOAT3 pos, XMFLOAT3 rot, float fov) : Camera(aR, pos, rot, fov, 0.1f, 200.0f) {}
Camera::Camera(float aR, XMFLOAT3 pos, XMFLOAT3 rot, float fov, float nearP, float farP) :
	aspectRatio(aR), 
	fov(fov), 
	nearPlane(nearP), 
	farPlane(farP),
	movementSpeed(5.0f),
	lookSensitivity(0.005f),
	isPerspective(true),
	projectionMatrix(XMFLOAT4X4()),
	viewMatrix(XMFLOAT4X4())
{
	this->transform.SetPosition(pos);
	this->transform.SetRotation(rot);
	UpdateProjectionMatrix(aR);
	UpdateViewMatrix();
}

Camera::~Camera()
{}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

float Camera::GetFOV()
{
	return fov;
}

Transform Camera::GetTransform()
{
	return transform;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), aspectRatio, nearPlane, farPlane);
	XMStoreFloat4x4(&projectionMatrix, projection);
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 direction = transform.GetForward();
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&direction), XMLoadFloat3(&worldUp));
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt)
{
#pragma region Keyboard Input
	if (Input::KeyDown('W')) { transform.MoveRelative(0.0f, 0.0f, movementSpeed * dt); } // Fprward
	if (Input::KeyDown('S')) { transform.MoveRelative(0.0f, 0.0f, -movementSpeed * dt); } // Backward
	if (Input::KeyDown('A')) { transform.MoveRelative(-movementSpeed * dt, 0.0f, 0.0f); } // Left
	if (Input::KeyDown('D')) { transform.MoveRelative(movementSpeed * dt, 0.0f, 0.0f); } // Right
	if (Input::KeyDown(VK_SPACE) || Input::KeyDown('E')) { transform.MoveRelative(0.0f, movementSpeed * dt, 0.0f); } // Up
	if (Input::KeyDown('X') || Input::KeyDown('Q')) { transform.MoveRelative(0.0f, -movementSpeed * dt, 0.0f); } // Down
	if (Input::KeyDown(VK_SHIFT)) { movementSpeed += 0.1f; } // Move Faster
	if (Input::KeyDown(VK_CONTROL) && movementSpeed > 0.1f) { movementSpeed -= 0.1f; } // Move Slower
#pragma endregion

#pragma region Mouse Input
	if (Input::MouseRightDown())
	{
		int cursorMovementX = Input::GetMouseXDelta();
		int cursorMovementY = Input::GetMouseYDelta();

		transform.Rotate(cursorMovementY * lookSensitivity, cursorMovementX * lookSensitivity, 0.0f);

		if (transform.GetPitchYawRoll().x > M_PI_2) { transform.SetRotation((float) M_PI_2 - 0.001f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z); }
		if (transform.GetPitchYawRoll().x < -M_PI_2) { transform.SetRotation((float) - M_PI_2 + 0.001f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z); }
	}
#pragma endregion

	UpdateViewMatrix();
}

bool Camera::RaycastFromMouse(DirectX::XMFLOAT3& rayOrigin)
{
	//// Source - https://stackoverflow.com/a/40116841
	//// Posted by Jack Reggin
	//// Retrieved 2026-04-23, License - CC BY-SA 3.0

	//DirectX::XMVECTOR mouseNear = DirectX::XMVectorSet((float)rayOrigin.x, (float)rayOrigin.y, 0.0f, 0.0f);
	//DirectX::XMVECTOR mouseFar = DirectX::XMVectorSet((float)rayOrigin.x, (float)rayOrigin.y, 1.0f, 0.0f);
	//DirectX::XMVECTOR unprojectedNear = DirectX::XMVector3Unproject(mouseNear, 0, 0, Window::Width(), Window::Height(), a_nearZ, a_farZ,
	//	a_projection, a_view, DirectX::XMMatrixIdentity());
	//DirectX::XMVECTOR unprojectedFar = DirectX::XMVector3Unproject(mouseFar, 0, 0, a_width, a_height, a_nearZ, a_farZ,
	//	a_projection, a_view, DirectX::XMMatrixIdentity());
	//DirectX::XMVECTOR result = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(unprojectedFar, unprojectedNear));
	//DirectX::XMFLOAT3 direction;
	//DirectX::XMStoreFloat3(&direction, result);
	//return direction;

	//Get the distance to the ground.
	//DirectX::XMFLOAT3 cameraPosition = a_camera.GetPosition();

	////Get the point on the ground.
	//cameraPosition.x += direction.x * (cameraPosition.y / -direction.y);
	//cameraPosition.z += direction.z * (cameraPosition.y / -direction.y);
	return true;
}
