#include "Actor.h"

Actor::Actor()
{
}

Actor::Actor(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
	transform = std::make_shared<Transform>();
}

Actor::Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform) : Actor(mesh)
{
	this->transform = transform;
}

Actor::~Actor()
{

}

void Actor::SetName(std::string name) { this->name = name; }
std::string Actor::GetName() { return name; }

std::shared_ptr<Mesh> Actor::GetMesh() { return mesh; }
std::shared_ptr<Transform> Actor::GetTransform() { return transform; }

void Actor::Draw()
{
	mesh->Draw();
}

