#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>

class Actor
{
public:
	Actor();
	Actor(std::shared_ptr<Mesh> mesh);
	Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform);
	~Actor();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	void Draw();

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

