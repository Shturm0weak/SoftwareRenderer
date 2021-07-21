#include "pch.h"
#include "Utils.h"

glm::vec3 Normalize(const glm::vec3& vector)
{
	float l = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return vector * (1.0f / l);
}

float Interpolate(float y1, float y2, float x)
{
	return y1 + x * ((y2 - y1) / 1.0f);
}

glm::vec3 Interpolate(const glm::vec3& y1, const glm::vec3& y2, float x)
{
	return { Interpolate(y1.x, y2.x, x), Interpolate(y1.y, y2.y, x), Interpolate(y1.z, y2.z, x) };
}

float Max(float a, float b)
{
	return a > b ? a : b;
}

glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n)
{
	return i - n * 2.0f * glm::dot(n, i);
}

glm::vec3 Clamp(const glm::vec3& a, const glm::vec3& b)
{
	glm::vec3 result = a;
	result.x = a.x > b.x ? b.x : a.x;
	result.y = a.y > b.y ? b.y : a.y;
	result.z = a.z > b.z ? b.z : a.z;
	return result;
}

glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end)
{
	planeN = Normalize(planeN);
	float plane_d = -glm::dot(planeN, planeP);
	float ad = glm::dot(start, planeN);
	float bd = glm::dot(end, planeN);
	float t = (-plane_d - ad) / (bd - ad);
	glm::vec3 lineStartToEnd = end - start;
	glm::vec3 lineToIntersect = lineStartToEnd * t;
	return start + lineToIntersect;
}
