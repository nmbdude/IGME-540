#pragma once
#include "Mesh.h"
#include "TypeDefs.h"
#include "Actor.h"
#include "PathHelpers.h"
#include "Material.h"
#include <memory>
#include <DirectXMath.h>

class Gizmo : Actor
{
public:
	Gizmo();
	~Gizmo();

	void Drag(DirectX::XMFLOAT3 dragAmount, AActor draggedArrow, AActor draggedActor);

private:
	std::shared_ptr<Transform> transform;

	AActor AYArrow = std::make_shared<Actor>();
	AActor AXArrow = std::make_shared<Actor>();
	AActor AZArrow = std::make_shared<Actor>();
};

