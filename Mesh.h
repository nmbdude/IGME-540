#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"

typedef Microsoft::WRL::ComPtr<ID3D11Buffer> ComPtrBuf;

class Mesh
{
	ComPtrBuf vertexBuffer;
	ComPtrBuf indexBuffer;
	int indexCount;
	int vertexCount;
	int triangleCount;

public:
	Mesh(int indexCount, int* indices, int vertexCount, Vertex* vertices);
	Mesh();
	~Mesh();
	ComPtrBuf GetVertexBuffer();
	ComPtrBuf GetIndexBuffer();
	int GetIndexCount();
	int GetVertexCount();
	int GetTriangleCount();
	void Draw();
};

