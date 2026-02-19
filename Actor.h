#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include <memory>
#include <string>

class Actor
{
public:
	Actor();
	Actor(std::shared_ptr<Mesh> mesh);
	Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform);
	~Actor();
	
	void SetName(std::string name);
	std::string GetName();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	void Draw();

private:
	std::string name;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

