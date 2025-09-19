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
XMVECTOR Object::GetPosition() const
{
	return XMVectorSet(m_position.r[3].m128_f32[0], m_position.r[3].m128_f32[1], m_position.r[3].m128_f32[2], 1.0f);
}

void Object::SetRotation(const XMVECTOR& rot)
{
	XMMATRIX rotX = XMMatrixRotationX(XMVectorGetX(rot));
	XMMATRIX rotY = XMMatrixRotationY(XMVectorGetY(rot));
	XMMATRIX rotZ = XMMatrixRotationZ(XMVectorGetZ(rot));
	m_rotation = rotZ * rotY * rotX;

	m_isDirty = true;
}
void Object::Rotate(const XMVECTOR& delta)
{
	XMMATRIX deltaX = XMMatrixRotationX(XMVectorGetX(delta));
	XMMATRIX deltaY = XMMatrixRotationY(XMVectorGetY(delta));
	XMMATRIX deltaZ = XMMatrixRotationZ(XMVectorGetZ(delta));
	m_rotation = m_rotation * (deltaZ * deltaY * deltaX);

	m_isDirty = true;
}
XMVECTOR Object::GetRotation() const // Not actually sure how this works
{
	XMFLOAT4X4 rotMatrix = {};
	XMStoreFloat4x4(&rotMatrix, m_rotation);
	XMFLOAT3 euler = {};
	euler.y = atan2f(rotMatrix._13, rotMatrix._33); // yaw
	float cosYaw = cosf(euler.y);

	if (fabsf(cosYaw) > 1e-6)
	{
		euler.x = asinf(-rotMatrix._23); // pitch
		euler.z = atan2f(rotMatrix._21, rotMatrix._22); // roll
	}
	else
	{
		euler.x = asinf(-rotMatrix._23); // pitch
		euler.z = 0.0f;
	}

	return XMVectorSet(euler.x, euler.y, euler.z, 0.0f);
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
			if (m_parent->m_isDirty) m_worldMatrix *= m_parent->GetWorldMatrix();
			else m_worldMatrix *= m_parent->m_worldMatrix;
		}

		m_isDirty = false;
		MakeChildDirty();
	}

	return m_worldMatrix;
}