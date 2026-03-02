#define TINYOBJLOADER_IMPLEMENTATION

#include "Mesh.h"
#include <string>
#include <memory>

Mesh::Mesh(const char* filePath)
{
	// This mesh loader is from the tinyobjloader documentation with adjustments to fit current architecture
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath, nullptr, true);

	if (!ret)
	{
		throw std::runtime_error(err);
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t indexOffset = 0;

		// Loop over faces
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];

				// Position
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				Vertex vertex = {};
				vertex.Position = { vx, vy, vz };

				// Normal
				if (idx.normal_index >= 0)
				{
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					vertex.normal = { nx, ny, nz };
				}

				// UV
				if (idx.texcoord_index >= 0)
				{
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					vertex.uv = { tx, ty };
				}

				vertices.push_back(vertex);
				indices.push_back(static_cast<unsigned int>(vertices.size() - 1));
			}
			indexOffset += fv;
		}
	}

	vertexCount = static_cast<int>(vertices.size());
	indexCount = static_cast<int>(indices.size());
	triangleCount = indexCount / 3;

	// Create vertex buffer
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices.data();
	Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

	// Create index buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(unsigned int));
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices.data();
	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
}

Mesh::Mesh(unsigned int indexCount, unsigned int* indices, unsigned int vertexCount, Vertex* vertices, const std::string& meshName)
{
	this->indexCount = indexCount;
	this->vertexCount = vertexCount;
	this->name = meshName;
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = vertexCount * sizeof(Vertex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices;
	Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

	
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	
	ibd.ByteWidth = indexCount * sizeof(unsigned int);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;

	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	triangleCount = indexCount / 3;
}


Mesh::Mesh()
{
	vertexCount = 1;
	indexCount = 3;
	vertexBuffer = ComPtrBuf();
	indexBuffer = ComPtrBuf();
	triangleCount = 1;
}

Mesh::~Mesh()
{

}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

int Mesh::GetIndexCount()
{
	return indexCount;
}

std::string Mesh::GetName()
{
	return name;
}

int Mesh::GetVertexCount()
{
	return vertexCount;
}

int Mesh::GetTriangleCount()
{
	return triangleCount;
}

void Mesh::Draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::Context->DrawIndexed(indexCount, 0, 0);
}
