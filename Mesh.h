#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include "Graphics.h"
#include <DirectXMath.h>
#include "Vertex.h"

typedef Microsoft::WRL::ComPtr<ID3D11Buffer> ComPtrBuf;

namespace COLORS
{
	using namespace DirectX;
	inline XMFLOAT4 RED = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	inline XMFLOAT4 GREEN = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	inline XMFLOAT4 BLUE = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	inline XMFLOAT4 WHITE = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	inline XMFLOAT4 BLACK = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
}

class Mesh
{
	ComPtrBuf vertexBuffer;
	ComPtrBuf indexBuffer;
	int indexCount;
	int vertexCount;
	int triangleCount;
	std::string name;

public:
	Mesh(unsigned int indexCount, unsigned int* indices, unsigned int vertexCount, Vertex* vertices, const std::string& meshName);
	Mesh();
	~Mesh();
	ComPtrBuf GetVertexBuffer();
	ComPtrBuf GetIndexBuffer();
	int GetIndexCount();
	std::string GetName();
	int GetVertexCount();
	int GetTriangleCount();
	void Draw();
};

