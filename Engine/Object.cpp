#include "Object.h"

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;
using namespace DirectX;

static UINT s_idCounter = 0;

void Object::MakeChildDirty()
{
	for (auto& child : m_childrens)
	{
		child->m_isDirty = true;
		child->MakeChildDirty();
	}
}

void Object::AddChild(Object* child)
{
	if (!child) return;

	if (child->m_parent)
	{
		child->m_parent->m_childrens.erase
		(
			remove
			(
				child->m_parent->m_childrens.begin(),
				child->m_parent->m_childrens.end(),
				child
			),
			child->m_parent->m_childrens.end()
		);
	}

	child->m_parent = this;
	m_childrens.emplace_back(child);

	MakeChildDirty();
}

Object::Object(Shapes shape) : m_shape(shape), m_id(s_idCounter++)
{
	VDGM::g_objects.emplace_back(this);

#ifdef _DEBUG
	cout << "Object created. ID: " << m_id << endl;
#endif
}

Object::~Object()
{
	auto it = find(VDGM::g_objects.begin(), VDGM::g_objects.end(), this);
	if (it != VDGM::g_objects.end()) VDGM::g_objects.erase(it);

#ifdef _DEBUG
	cout << "Object destroyed. ID: " << m_id << endl;
#endif
}

void Object::SetPosition(const XMVECTOR& pos)
{
	m_position = XMMatrixTranslationFromVector(pos);
	m_isDirty = true;
}
void Object::MovePosition(const XMVECTOR& delta)
{
	XMVECTOR currentPos = XMVectorSet(m_position.r[3].m128_f32[0], m_position.r[3].m128_f32[1], m_position.r[3].m128_f32[2], 1.0f);
	XMVECTOR newPos = XMVectorAdd(currentPos, delta);
	m_position = XMMatrixTranslationFromVector(newPos);

	m_isDirty = true;
}
void Object::MoveDirection(Directions dir, float distance)
{
	XMVECTOR forward = XMVectorSet(m_rotation.r[2].m128_f32[0], m_rotation.r[2].m128_f32[1], m_rotation.r[2].m128_f32[2], 0.0f);
	XMVECTOR right = XMVectorSet(m_rotation.r[0].m128_f32[0], m_rotation.r[0].m128_f32[1], m_rotation.r[0].m128_f32[2], 0.0f);
	XMVECTOR up = XMVectorSet(m_rotation.r[1].m128_f32[0], m_rotation.r[1].m128_f32[1], m_rotation.r[1].m128_f32[2], 0.0f);
	XMVECTOR moveDelta = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	switch (dir)
	{
	case Directions::Forward:
		moveDelta = XMVectorScale(forward, distance);
		break;

	case Directions::Backward:
		moveDelta = XMVectorScale(forward, -distance);
		break;

	case Directions::Right:
		moveDelta = XMVectorScale(right, distance);
		break;

	case Directions::Left:
		moveDelta = XMVectorScale(right, -distance);
		break;

	case Directions::Up:
		moveDelta = XMVectorScale(up, distance);
		break;

	case Directions::Down:
		moveDelta = XMVectorScale(up, -distance);
		break;

	default:
		break;
	}

	MovePosition(moveDelta);
}
XMVECTOR Object::GetPosition() const
{
	return XMVectorSet(m_position.r[3].m128_f32[0], m_position.r[3].m128_f32[1], m_position.r[3].m128_f32[2], 1.0f);
}

void Object::SetRotation(const XMVECTOR& rot)
{
	m_pitchYawRoll = rot;

	XMMATRIX rotX = XMMatrixRotationX(XMVectorGetX(m_pitchYawRoll));
	XMMATRIX rotY = XMMatrixRotationY(XMVectorGetY(m_pitchYawRoll));
	XMMATRIX rotZ = XMMatrixRotationZ(XMVectorGetZ(m_pitchYawRoll));
	m_rotation = rotZ * rotY * rotX;

	m_isDirty = true;
}
void Object::Rotate(const XMVECTOR& delta)
{
	m_pitchYawRoll = XMVectorAdd(m_pitchYawRoll, delta);

	XMMATRIX rotX = XMMatrixRotationX(XMVectorGetX(m_pitchYawRoll));
	XMMATRIX rotY = XMMatrixRotationY(XMVectorGetY(m_pitchYawRoll));
	XMMATRIX rotZ = XMMatrixRotationZ(XMVectorGetZ(m_pitchYawRoll));
	m_rotation = rotZ * rotY * rotX;

	m_isDirty = true;
}
XMVECTOR Object::GetRotation() const
{
	return m_pitchYawRoll;
}

void Object::SetScale(const XMVECTOR& scl)
{
	XMFLOAT3 scale = {};
	XMStoreFloat3(&scale, scl);
	m_scale = XMMatrixScaling(scale.x, scale.y, scale.z);

	m_isDirty = true;
}
void Object::Scale(const XMVECTOR& factor)
{
	XMFLOAT4X4 currentScale = {};
	XMStoreFloat4x4(&currentScale, m_scale);

	XMFLOAT3 scaleFactor = {};
	XMStoreFloat3(&scaleFactor, factor);
	currentScale._11 *= scaleFactor.x;
	currentScale._22 *= scaleFactor.y;
	currentScale._33 *= scaleFactor.z;

	m_scale = XMLoadFloat4x4(&currentScale);

	m_isDirty = true;
}
XMVECTOR Object::GetScale() const
{
	XMFLOAT4X4 scaleMatrix = {};
	XMStoreFloat4x4(&scaleMatrix, m_scale);

	return XMVectorSet(scaleMatrix._11, scaleMatrix._22, scaleMatrix._33, 0.0f);
}

XMMATRIX Object::GetWorldMatrix()
{
	if (m_isDirty)
	{
		m_worldMatrix = m_scale * m_rotation * m_position;

		if (m_parent)
		{
			if (!m_ignorePosition && !m_ignoreRotation && !m_ignoreScale)
			{
				if (m_parent->m_isDirty) m_worldMatrix *= m_parent->GetWorldMatrix();
				else m_worldMatrix *= m_parent->m_worldMatrix;
			}
			else
			{
				XMMATRIX parentMatrix = m_parent->GetWorldMatrix();

				if (m_ignorePosition)
				{
					if (!(m_ignorePosition % IgnoreParentAxis::X)) parentMatrix.r[3].m128_f32[0] = 0.0f;
					if (!(m_ignorePosition % IgnoreParentAxis::Y)) parentMatrix.r[3].m128_f32[1] = 0.0f;
					if (!(m_ignorePosition % IgnoreParentAxis::Z)) parentMatrix.r[3].m128_f32[2] = 0.0f;
				}
				if (m_ignoreRotation)
				{
				}
				if (m_ignoreScale)
				{
					if (!(m_ignoreScale % IgnoreParentAxis::X)) parentMatrix.r[0].m128_f32[0] = 1.0f;
					if (!(m_ignoreScale % IgnoreParentAxis::Y)) parentMatrix.r[1].m128_f32[1] = 1.0f;
					if (!(m_ignoreScale % IgnoreParentAxis::Z)) parentMatrix.r[2].m128_f32[2] = 1.0f;
				}

				m_worldMatrix *= parentMatrix;
			}
		}

		m_isDirty = false;
		MakeChildDirty();
	}

	return m_worldMatrix;
}