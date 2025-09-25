#include "Object.h"

using namespace std;
using namespace DirectX;

vector<Object*> g_objects;

static UINT s_idCounter = 0;

void Object::SetDirty()
{
	if (m_isDirty) return;

	m_isDirty = true;
	for (auto& child : m_childrens) child->SetDirty();
}

void Object::AddChild(Object* child)
{
	if (!child) return;

	if (child->m_parent) child->m_parent->RemoveChild(child);

	child->m_parent = this;
	m_childrens.emplace_back(child);

	SetDirty();
}

void Object::RemoveChild(Object* child)
{
	if (!child) return;

	auto it = find(m_childrens.begin(), m_childrens.end(), child);
	if (it != m_childrens.end())
	{
		m_childrens.erase(it);
		child->m_parent = nullptr;
	}
}

Object::Object(Shapes shape, VertexShaders vertexShader, PixelShaders pixelShader) : m_shape(shape), m_vertexShader(vertexShader), m_pixelShader(pixelShader), m_id(s_idCounter++)
{
	if (shape != Shapes::None) g_objects.emplace_back(this);

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
	m_positionMatrix = XMMatrixTranslationFromVector(m_position);

	if (!m_isDirty) SetDirty();
}
void Object::MovePosition(const XMVECTOR& delta)
{
	m_position = XMVectorAdd(m_position, delta);
	m_positionMatrix = XMMatrixTranslationFromVector(m_position);

	if (!m_isDirty) SetDirty();
}
void Object::MoveDirection(Directions dir, float distance)
{
	const XMVECTOR forward = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), m_rotationMatrix));
	const XMVECTOR right = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), m_rotationMatrix));
	const XMVECTOR up = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_rotationMatrix));

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

	if (!m_isDirty) SetDirty();
}
XMVECTOR Object::GetWorldPosition() const
{
	if (m_isDirty) GetWorldMatrix();

	return XMVectorSet(m_worldMatrix.r[3].m128_f32[0], m_worldMatrix.r[3].m128_f32[1], m_worldMatrix.r[3].m128_f32[2], 1.0f);
}

void Object::SetRotation(const XMVECTOR& rot)
{
	m_rotation = rot;
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
void Object::Rotate(const XMVECTOR& delta)
{
	m_rotation = XMVectorAdd(m_rotation, delta);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
void Object::LookAt(const XMVECTOR& target) // I have no idea how the hell this works
{
	XMVECTOR toTarget = XMVectorSubtract(target, GetWorldPosition());
	if (XMVector3LessOrEqual(XMVector3LengthSq(toTarget), XMVectorReplicate(1e-8f))) return; // To avoid NaN errors

	XMVECTOR direction = XMVector3Normalize(toTarget);

	const float dirX = XMVectorGetX(direction);
	const float dirY = XMVectorGetY(direction);
	const float dirZ = XMVectorGetZ(direction);

	float pitch = asinf(clamp(-dirY, -1.0f, 1.0f));
	float yaw = atan2f(dirX, dirZ);
	float roll = 0.0f;

	m_rotation = XMVectorSet(pitch, yaw, roll, 0.0f);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
XMVECTOR Object::GetWorldRotation() const // This one too
{
	if (m_isDirty) GetWorldMatrix();

	XMVECTOR scale, rotationQuat, translation;
	XMMatrixDecompose(&scale, &rotationQuat, &translation, m_worldMatrix);

	XMFLOAT4 quat = {};
	XMStoreFloat4(&quat, rotationQuat);

	// This is to prevent gimbal lock
	const float test = quat.x * quat.y + quat.z * quat.w;
	if (test > 0.499f)
	{
		float yaw = 2.0f * atan2f(quat.x, quat.w);
		float pitch = XM_PIDIV2;
		float roll = 0.0f;
		return XMVectorSet(roll, pitch, yaw, 0.0f);
	}
	if (test < -0.499f)
	{
		float yaw = -2.0f * atan2f(quat.x, quat.w);
		float pitch = -XM_PIDIV2;
		float roll = 0.0f;
		return XMVectorSet(roll, pitch, yaw, 0.0f);
	}

	float roll = atan2f
	(
		2.0f * (quat.w * quat.x + quat.y * quat.z),
		1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y)
	);
	float pitch = asinf(2.0f * (quat.w * quat.y - quat.z * quat.x));
	float yaw = atan2f
	(
		2.0f * (quat.w * quat.z + quat.x * quat.y),
		1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z)
	);

	return XMVectorSet(roll, pitch, yaw, 0.0f);
}
XMVECTOR Object::GetWorldDirection(Directions dir) const
{
	if (m_isDirty) GetWorldMatrix();
	XMVECTOR direction = XMVectorZero();

	switch (dir)
	{
	case Directions::Forward: direction = XMVector3Normalize(XMVectorSet(m_worldMatrix.r[2].m128_f32[0], m_worldMatrix.r[2].m128_f32[1], m_worldMatrix.r[2].m128_f32[2], 0.0f)); break;
	case Directions::Backward: direction = XMVector3Normalize(XMVectorSet(-m_worldMatrix.r[2].m128_f32[0], -m_worldMatrix.r[2].m128_f32[1], -m_worldMatrix.r[2].m128_f32[2], 0.0f)); break;

	case Directions::Right: direction = XMVector3Normalize(XMVectorSet(m_worldMatrix.r[0].m128_f32[0], m_worldMatrix.r[0].m128_f32[1], m_worldMatrix.r[0].m128_f32[2], 0.0f)); break;
	case Directions::Left: direction = XMVector3Normalize(XMVectorSet(-m_worldMatrix.r[0].m128_f32[0], -m_worldMatrix.r[0].m128_f32[1], -m_worldMatrix.r[0].m128_f32[2], 0.0f)); break;

	case Directions::Up: direction = XMVector3Normalize(XMVectorSet(m_worldMatrix.r[1].m128_f32[0], m_worldMatrix.r[1].m128_f32[1], m_worldMatrix.r[1].m128_f32[2], 0.0f)); break;
	case Directions::Down: direction = XMVector3Normalize(XMVectorSet(-m_worldMatrix.r[1].m128_f32[0], -m_worldMatrix.r[1].m128_f32[1], -m_worldMatrix.r[1].m128_f32[2], 0.0f)); break;

	default: break;
	}

	return direction;
}

void Object::SetScale(const XMFLOAT3& scl)
{
	m_scale = scl;
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	if (!m_isDirty) SetDirty();
}
void Object::Scale(const XMFLOAT3& factor)
{
	m_scale.x *= factor.x;
	m_scale.y *= factor.y;
	m_scale.z *= factor.z;
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	if (!m_isDirty) SetDirty();
}

XMFLOAT3 Object::GetWorldScale() const
{
	if (m_isDirty) GetWorldMatrix();

	XMVECTOR scale;
	XMVECTOR rotationQuat;
	XMVECTOR translation;
	XMMatrixDecompose(&scale, &rotationQuat, &translation, m_worldMatrix);

	XMFLOAT3 scl = {};
	XMStoreFloat3(&scl, scale);

	return scl;
}

XMMATRIX Object::GetWorldMatrix() const
{
	if (m_isDirty)
	{
		m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

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
	}

	return m_worldMatrix;
}