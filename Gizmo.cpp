#include "Gizmo.h"

using namespace DirectX;

Gizmo::Gizmo()
{
	std::shared_ptr<Mesh> SM_GizmoArrow = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Arrow.obj").c_str());
	
	Mat M_Red = std::make_shared<Material>("VertexShader.cso", "PixelShader.cso");
	Mat M_Green = std::make_shared<Material>("VertexShader.cso", "PixelShader.cso");
	Mat M_Blue = std::make_shared<Material>("VertexShader.cso", "PixelShader.cso");

	M_Red->SetColorTint(XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	M_Green->SetColorTint(XMFLOAT4(0.f, 1.f, 0.f, 1.f));
	M_Blue->SetColorTint(XMFLOAT4(0.f, 0.f, 1.f, 1.f));

	// Up Arrow
	AActor AYArrow = std::make_shared<Actor>(SM_GizmoArrow);
	AYArrow->SetMaterial(M_Green);

	AActor AXArrow = std::make_shared<Actor>(SM_GizmoArrow);
	AXArrow->SetMaterial(M_Red);
	AXArrow->GetTransform()->SetRotation(XMFLOAT3(1.f, 0.f, 1.f));

	AActor AZArrow = std::make_shared<Actor>(SM_GizmoArrow);
	AZArrow->SetMaterial(M_Blue);
	AZArrow->GetTransform()->SetRotation(XMFLOAT3(0.f, 0.f, 1.f));
}

Gizmo::~Gizmo()
{
}
