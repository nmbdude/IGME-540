#include "Transform.h"

using namespace DirectX;

Transform::Transform() : 
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1)
{}

void Transform::MoveAbs(float x, float y, float z)
{
	XMVECTOR posVec = XMLoadFloat3(&position);
	posVec += XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, posVec);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR rotVec = XMLoadFloat3(&rotation);
	rotVec += XMVectorSet(pitch, yaw, roll, 0);
	XMStoreFloat3(&rotation, rotVec);
}

void Transform::Scale(float x, float y, float z)
{
	XMVECTOR scaleVec = XMLoadFloat3(&scale);
	scaleVec *= XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&scale, scaleVec);
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(dirty)
	{
		XMMATRIX tMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
		XMMATRIX sMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		XMMATRIX worldMat = sMat * rMat * tMat;

		XMStoreFloat4x4(&worldMatrix, worldMat);
		dirty = false;
		return worldMatrix;
	}
}
