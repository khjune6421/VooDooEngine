#include "WindMill.h"

using namespace std;
using namespace DirectX;

WindMill::WindMill(Shapes shape) : Object(shape)
{
	m_windmillWing = make_unique<Object>(Shapes::WindmillWing);
	m_windmillWing->SetParent(this);
	m_windmillWing->SetPosition({ 0.0f, 2.0f, 0.0f });
}

void WindMill::Update(float deltaTime)
{
	m_windmillWing->Rotate({ 0.0f, 0.0f, 2.0f * deltaTime });

	SimpleMath::Vector3 moveDelta = { 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('W') & 0x8000) moveDelta.z += m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('S') & 0x8000) moveDelta.z -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('A') & 0x8000) moveDelta.x -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('D') & 0x8000) moveDelta.x += m_moveSpeed * deltaTime;

	MovePosition(moveDelta);
}