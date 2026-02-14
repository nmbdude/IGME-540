#include "Transform.h"


using namespace DirectX;

Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
}

DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return rotation; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (dirty)
	{
		XMMATRIX tMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
		XMMATRIX sMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		XMMATRIX worldMat = sMat * rMat * tMat;

		XMStoreFloat4x4(&worldMatrix, worldMat);
		dirty = false;
	}
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	XMFLOAT4X4 wITM = {};
	if(dirty)
	{
		XMMATRIX tMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
		XMMATRIX sMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		XMMATRIX worldMat = sMat * rMat * tMat;

		XMStoreFloat4x4(&wITM, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));
	}
	return wITM;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	dirty = true;
	XMVECTOR posVec = XMLoadFloat3(&position);
	posVec = XMVectorSet(x, y, z, 0);
	posVec += XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, posVec);
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	dirty = true;
	XMVECTOR posVec = XMLoadFloat3(&position);
	posVec += XMLoadFloat3(&offset);
	XMStoreFloat3(&position, posVec);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	dirty = true;
	XMVECTOR rotVec = XMLoadFloat3(&rotation);
	rotVec += XMVectorSet(pitch, yaw, roll, 0);
	XMStoreFloat3(&rotation, rotVec);
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	dirty = true;
	XMVECTOR rotVec = XMLoadFloat3(&this->rotation);
	rotVec += XMLoadFloat3(&rotation);
	XMStoreFloat3(&this->rotation, rotVec);
}

void Transform::Scale(float x, float y, float z)
{
	dirty = true;
	XMVECTOR scaleVec = XMLoadFloat3(&scale);
	scaleVec *= XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&scale, scaleVec);
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	dirty = true;
	XMVECTOR scaleVec = XMLoadFloat3(&this->scale);
	scaleVec *= XMLoadFloat3(&scale);
	XMStoreFloat3(&this->scale, scaleVec);
}

void Transform::SetPosition(float x, float y, float z)
{
	dirty = true;
	position.x = x;
	position.y = y;
	position.z = z;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	dirty = true;
	this->position = position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	dirty = true;
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	dirty = true;
	this->rotation = rotation;
}

void Transform::SetScale(float x, float y, float z)
{
	dirty = true;
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	dirty = true;
	this->scale = scale;
}

