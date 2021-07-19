#include "pch.h"
#include "Utils.h"

glm::vec3 Normalize(glm::vec3 vector)
{
	float l = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	vector.x /= l; vector.y /= l; vector.z /= l;
	return vector;
}

float InterpolateBetweenTwoDots(float y1, float y2, float x)
{
	return y1 + x * ((y2 - y1) / 1.0f);
}
