#include "Actor.h"

Actor::Actor() {}
	//: Actor(std::make_shared<Mesh>(), std::make_shared<Transform>(), std::make_shared<Material>()) {}

Actor::Actor(std::shared_ptr<Mesh> mesh) 
	: Actor(mesh, std::make_shared<Transform>(), std::make_shared<Material>()) {}

Actor::Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform) 
	: Actor(mesh, transform, std::make_shared<Material>()) {}

Actor::Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
	: Actor(mesh, std::make_shared<Transform>(), material) {}

Actor::Actor(std::shared_ptr<Mesh> mesh, std::shared_ptr<Transform> transform, std::shared_ptr<Material> material) 
	: mesh(mesh), transform(transform), material(material) {}

Actor::~Actor() {}

void Actor::SetName(std::string name) { this->name = name; }
void Actor::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }

std::string Actor::GetName() { return name; }
std::shared_ptr<Mesh> Actor::GetMesh() { return mesh; }
std::shared_ptr<Transform> Actor::GetTransform() { return transform; }
std::shared_ptr<Material> Actor::GetMaterial() { return material; }

void Actor::Draw()
{
	Graphics::Context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
	Graphics::Context->PSSetShader(material->GetPixelShader().Get(), 0, 0);
	mesh->Draw();
}

