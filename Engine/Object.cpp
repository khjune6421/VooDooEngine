#include "Object.h"

using namespace std;
using namespace DirectX;

vector<Object*> g_objects;

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

Object::Object(Shapes shape, VertexShaders vertexShader, PixelShaders pixelShader) : m_shape(shape), m_vertexShader(vertexShader), m_pixelShader(pixelShader), m_id(s_idCounter++)
{
	g_objects.emplace_back(this);

#ifdef _DEBUG
	cout << "Object created. ID: " << m_id << endl;
#endif
}

Object::~Object()
{
	auto it = find(g_objects.begin(), g_objects.end(), this);
	if (it != g_objects.end()) g_objects.erase(it);

#ifdef _DEBUG
	cout << "Object destroyed. ID: " << m_id << endl;
#endif
}

void Object::SetPosition(const XMVECTOR& pos)
{
	m_position = pos;
	m_positionMatRix = XMMatrixTranslationFromVector(m_position);
	m_isDirty = true;
}
void Object::MovePosition(const XMVECTOR& delta)
{
	m_position = XMVectorAdd(m_position, delta);
	m_positionMatRix = XMMatrixTranslationFromVector(m_position);

	m_isDirty = true;
}
void Object::MoveDirection(Directions dir, float distance)
{
	const XMVECTOR forward = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), m_rotationMatRix));
	const XMVECTOR right = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), m_rotationMatRix));
	const XMVECTOR up = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_rotationMatRix));

	XMVECTOR moveDelta = XMVectorZero();

	switch (dir)
	{
	case Directions::Forward: moveDelta = XMVectorScale(forward, distance); break;
	case Directions::Backward: moveDelta = XMVectorScale(forward, -distance); break;

	case Directions::Right: moveDelta = XMVectorScale(right, distance); break;
	case Directions::Left: moveDelta = XMVectorScale(right, -distance); break;

	case Directions::Up: moveDelta = XMVectorScale(up, distance); break;
	case Directions::Down: moveDelta = XMVectorScale(up, -distance); break;
	default: break;
	}

	MovePosition(moveDelta);
}

void Object::SetRotation(const XMVECTOR& rot)
{
	m_rotation = rot;

	XMMATRIX rotX = XMMatrixRotationX(XMVectorGetX(m_rotation));
	XMMATRIX rotY = XMMatrixRotationY(XMVectorGetY(m_rotation));
	XMMATRIX rotZ = XMMatrixRotationZ(XMVectorGetZ(m_rotation));
	m_rotationMatRix = rotZ * rotY * rotX;

	m_isDirty = true;
}
void Object::Rotate(const XMVECTOR& delta)
{
	m_rotation = XMVectorAdd(m_rotation, delta);

	XMMATRIX rotX = XMMatrixRotationX(XMVectorGetX(m_rotation));
	XMMATRIX rotY = XMMatrixRotationY(XMVectorGetY(m_rotation));
	XMMATRIX rotZ = XMMatrixRotationZ(XMVectorGetZ(m_rotation));
	m_rotationMatRix = rotZ * rotY * rotX;

	m_isDirty = true;
}

void Object::SetScale(const XMFLOAT3& scl)
{
	m_scale = scl;
	m_scaleMatRix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_isDirty = true;
}
void Object::Scale(const XMFLOAT3& factor)
{
	m_scale.x *= factor.x;
	m_scale.y *= factor.y;
	m_scale.z *= factor.z;
	m_scaleMatRix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_isDirty = true;
}

XMMATRIX Object::GetWorldMatrix()
{
	if (m_isDirty)
	{
		m_worldMatrix = m_scaleMatRix * m_rotationMatRix * m_positionMatRix;

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
					// not sure how I should implement this
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