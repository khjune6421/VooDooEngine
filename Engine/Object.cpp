#include "Object.h"

using namespace std;

Object::Object(Shapes shape) : m_shape(shape)
{
	g_objects.emplace_back(this);
}

Object::~Object()
{
	auto it = find(g_objects.begin(), g_objects.end(), this);
	if (it != g_objects.end()) g_objects.erase(it);
}