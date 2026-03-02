#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include <memory>
#include "BufferStructs.h"
#include "Material.h"
#include <string>

class Actor
{
public:
	Actor();
	Actor(std::shared_ptr<Mesh> mesh);
	Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform);
	Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform, std::shared_ptr<Material> material);
	~Actor();
	
	void SetName(std::string name);
	void SetMaterial(std::shared_ptr<Material> material);

	std::string GetName();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	void Draw();

private:
	std::string name;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

