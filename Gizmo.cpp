#include "Gizmo.h"

using namespace DirectX;

Gizmo::Gizmo()
{
	std::shared_ptr<Mesh> SM_GizmoArrow = std::make_shared<Mesh>("Assets/Meshes/Arrow.obj");
	
	Mat M_Red = std::make_shared<Material>(L"VertexShader.cso", L"PixelShader.cso");
	Mat M_Green = std::make_shared<Material>(L"VertexShader.cso", L"PixelShader.cso");
	Mat M_Blue = std::make_shared<Material>(L"VertexShader.cso", L"PixelShader.cso");

	M_Red->SetColorTint(XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	M_Green->SetColorTint(XMFLOAT4(0.f, 1.f, 0.f, 1.f));
	M_Blue->SetColorTint(XMFLOAT4(0.f, 0.f, 1.f, 1.f));

	transform = std::make_shared<Transform>();

	// Up Arrow
	AYArrow->SetMesh(SM_GizmoArrow);
	AYArrow->SetMaterial(M_Green);

	// Right Arrow
	AXArrow->SetMaterial(M_Red);
	AXArrow->GetTransform()->SetRotation(XMFLOAT3(1.f, 0.f, 1.f));

	// Forward Arrow
	AZArrow->SetMaterial(M_Blue);
	AZArrow->GetTransform()->SetRotation(XMFLOAT3(0.f, 0.f, 1.f));

	
}

Gizmo::~Gizmo()
{
}

void Gizmo::Drag(DirectX::XMFLOAT3 dragAmount, AActor draggedArrow, AActor draggedActor)
{
	// Determine which arrow is being dragged and apply the appropriate transformation to the dragged actor
	if (draggedArrow == AYArrow)
	{
		draggedActor->GetTransform()->MoveAbsolute(XMFLOAT3(0.f, dragAmount.y, 0.f));
	}
	else if (draggedArrow == AXArrow)
	{
		draggedActor->GetTransform()->MoveAbsolute(XMFLOAT3(dragAmount.x, 0.f, 0.f));
	}
	else if (draggedArrow == AZArrow)
	{
		draggedActor->GetTransform()->MoveAbsolute(XMFLOAT3(0.f, 0.f, dragAmount.z));
	}
}
