#pragma once
#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	void MoveAbs(float x, float y, float z);
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);

	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float x, float y, float z);

	DirectX::XMFLOAT4X4 GetWorldMatrix();

private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	bool dirty = true;
	DirectX::XMFLOAT4X4 worldMatrix;
};

