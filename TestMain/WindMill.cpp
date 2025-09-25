#include "WindMill.h"

using namespace std;
using namespace DirectX;

WindMill::WindMill(Shapes shape, UINT wingAmount) : Object(shape)
{
	m_windmillWing = make_unique<WindMillWing>();
	AddChild(m_windmillWing.get());
	m_windmillWing->MovePosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
}

void WindMill::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	XMVECTOR moveDelta = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	if (GetAsyncKeyState('W') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	if (GetAsyncKeyState('S') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	if (GetAsyncKeyState('A') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
	if (GetAsyncKeyState('D') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

	XMVECTOR normMove = XMVector3Normalize(moveDelta);
	normMove *= m_moveSpeed * deltaTime;
	MovePosition(normMove);
}