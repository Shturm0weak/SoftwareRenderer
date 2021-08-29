#include "pch.h"
#include "Utils.h"

inline glm::vec3 Normalize(const glm::vec3& vector)
{
	float l = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return vector * (1.0f / l);
}

inline float Lerp(const float& y1, const float& y2, const float& x)
{
	return y1 + x * (y2 - y1);
}

inline glm::vec2 Lerp(const glm::vec2& y1, const glm::vec2& y2, float x)
{
	return { Lerp(y1.x, y2.x, x), Lerp(y1.y, y2.y, x) };
}

inline glm::vec3 Lerp(const glm::vec3& y1, const glm::vec3& y2, float x)
{
	return { Lerp(y1.x, y2.x, x), Lerp(y1.y, y2.y, x), Lerp(y1.z, y2.z, x) };
}

inline glm::vec4 Lerp(const glm::vec4& y1, const glm::vec4& y2, float x)
{
	return { Lerp(y1.x, y2.x, x), Lerp(y1.y, y2.y, x), Lerp(y1.z, y2.z, x), Lerp(y1.w, y2.w, x ) };
}

inline glm::ivec3 Lerp(const glm::ivec3& y1, const glm::ivec3& y2, float x)
{
	return { Lerp(y1.x, y2.x, x), Lerp(y1.y, y2.y, x), Lerp(y1.z, y2.z, x) };
}

float Max(float a, float b)
{
	return a > b ? a : b;
}

glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n)
{
	return i - n * 2.0f * glm::dot(n, i);
}

glm::vec3 Clamp(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	glm::vec3 result = a;
	result.x = result.x > c.x ? c.x : result.x;
	result.y = result.y > c.y ? c.y : result.y;
	result.z = result.z > c.z ? c.z : result.z;
	result.x = result.x < b.x ? b.x : result.x;
	result.y = result.y < b.y ? b.y : result.y;
	result.z = result.z < b.z ? b.z : result.z;
	return result;
}

glm::vec2 Clamp(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	glm::vec2 result = a;
	result.x = result.x > c.x ? c.x : result.x;
	result.y = result.y > c.y ? c.y : result.y;
	result.x = result.x < b.x ? b.x : result.x;
	result.y = result.y < b.y ? b.y : result.y;
	return result;
}

inline glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end, float& t)
{
	planeN = Normalize(planeN);
	float planeD = -glm::dot(planeN, planeP);
	float ad = glm::dot(start, planeN);
	float bd = glm::dot(end, planeN);
	t = (-planeD - ad) / (bd - ad);
	glm::vec3 lineStartToEnd = end - start;
	glm::vec3 lineToIntersect = lineStartToEnd * t;
	return start + lineToIntersect;
}

int TriangleClipAgainstPlane(glm::vec3 planeP, glm::vec3 planeN, sr::TriangleV& inTri, sr::TriangleV& outTri1, sr::TriangleV& outTri2, bool screenSpace)
{
	planeN = Normalize(planeN);

	auto dist = [&](glm::vec3& p)
	{
		glm::vec3 n = Normalize(p);
		return (planeN.x * p.x + planeN.y * p.y + planeN.z * p.z - glm::dot(planeN, planeP));
	};

	sr::Vertex* inside_points[3];  int nInsidePointCount = 0;
	sr::Vertex *outside_points[3]; int nOutsidePointCount = 0;

	float d0 = dist(inTri.vertices[0].m_WorldPos);
	float d1 = dist(inTri.vertices[1].m_WorldPos);
	float d2 = dist(inTri.vertices[2].m_WorldPos);

	if (d0 >= 0)
	{
		inside_points[nInsidePointCount++] = &inTri.vertices[0];
	}
	else 
	{ 
		outside_points[nOutsidePointCount++] = &inTri.vertices[0];
	}
	if (d1 >= 0)
	{
		inside_points[nInsidePointCount++] = &inTri.vertices[1];
	}
	else
	{
		outside_points[nOutsidePointCount++] = &inTri.vertices[1];
	}
	if (d2 >= 0)
	{ 
		inside_points[nInsidePointCount++] = &inTri.vertices[2];
	}
	else
	{ 
		outside_points[nOutsidePointCount++] = &inTri.vertices[2];
	}

	if (nInsidePointCount == 0)
	{
		return 0;
	}

	if (nInsidePointCount == 3)
	{
		outTri1 = inTri;
		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		float t1, t2;
		outTri1.vertices[1].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[0]->m_WorldPos, outside_points[0]->m_WorldPos, t1);
		outTri1.vertices[2].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[0]->m_WorldPos, outside_points[1]->m_WorldPos, t2);
		if (screenSpace)
		{
			outTri1.vertices[0].m_P = inside_points[0]->m_P;
			outTri1.vertices[1].m_P = Lerp(inside_points[0]->m_P, outside_points[0]->m_P, t1);
			outTri1.vertices[2].m_P = Lerp(inside_points[0]->m_P, outside_points[1]->m_P, t2);
		}
		outTri1.vertices[1].m_Normal = Lerp(inside_points[0]->m_Normal, outside_points[0]->m_Normal, t1);
		outTri1.vertices[2].m_Normal = Lerp(inside_points[0]->m_Normal, outside_points[1]->m_Normal, t2);
		outTri1.vertices[1].m_C = Lerp(inside_points[0]->m_C, outside_points[0]->m_C, t1);
		outTri1.vertices[2].m_C = Lerp(inside_points[0]->m_C, outside_points[1]->m_C, t2);
		outTri1.vertices[1].m_UV = Lerp(inside_points[0]->m_UV, outside_points[0]->m_UV, t1);
		outTri1.vertices[2].m_UV = Lerp(inside_points[0]->m_UV, outside_points[1]->m_UV, t2);

		outTri1.vertices[0].m_WorldPos = inside_points[0]->m_WorldPos;
		outTri1.vertices[0].m_Normal = inside_points[0]->m_Normal;
		outTri1.vertices[0].m_C = inside_points[0]->m_C;
		outTri1.vertices[0].m_UV = inside_points[0]->m_UV;

		return 1;
	}
	else if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		float t1, t2;
		outTri1.vertices[2].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[0]->m_WorldPos, outside_points[0]->m_WorldPos, t1);
		outTri2.vertices[2].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[1]->m_WorldPos, outside_points[0]->m_WorldPos, t2);
		if (screenSpace)
		{
			outTri1.vertices[0].m_P = inside_points[0]->m_P;
			outTri1.vertices[1].m_P = inside_points[1]->m_P;
			outTri1.vertices[2].m_P = Lerp(inside_points[0]->m_P, outside_points[0]->m_P, t1);

			outTri2.vertices[0].m_P = inside_points[1]->m_P;
			outTri2.vertices[1].m_P = outTri1.vertices[2].m_P;
			outTri2.vertices[2].m_P = Lerp(inside_points[1]->m_P, outside_points[0]->m_P, t2);
		}
		outTri1.vertices[2].m_Normal = Lerp(inside_points[0]->m_Normal, outside_points[0]->m_Normal, t1);
		outTri2.vertices[2].m_Normal = Lerp(inside_points[1]->m_Normal, outside_points[0]->m_Normal, t2);
		outTri1.vertices[2].m_C = Lerp(inside_points[0]->m_C, outside_points[0]->m_C, t1);
		outTri2.vertices[2].m_C = Lerp(inside_points[1]->m_C, outside_points[0]->m_C, t2);
		outTri1.vertices[2].m_UV = Lerp(inside_points[0]->m_UV, outside_points[0]->m_UV, t1);
		outTri2.vertices[2].m_UV = Lerp(inside_points[1]->m_UV, outside_points[0]->m_UV, t2);

		outTri1.vertices[0].m_WorldPos = inside_points[0]->m_WorldPos;
		outTri1.vertices[1].m_WorldPos = inside_points[1]->m_WorldPos;
		outTri1.vertices[0].m_Normal = inside_points[0]->m_Normal;
		outTri1.vertices[1].m_Normal = inside_points[1]->m_Normal;
		outTri1.vertices[0].m_C = inside_points[0]->m_C;
		outTri1.vertices[1].m_C = inside_points[1]->m_C;
		outTri1.vertices[0].m_UV = inside_points[0]->m_UV;
		outTri1.vertices[1].m_UV = inside_points[1]->m_UV;

		outTri2.vertices[0].m_WorldPos = inside_points[1]->m_WorldPos;
		outTri2.vertices[1].m_WorldPos = outTri1.vertices[2].m_WorldPos;
		outTri2.vertices[0].m_Normal = inside_points[1]->m_Normal;
		outTri2.vertices[1].m_Normal = outTri1.vertices[2].m_Normal;
		outTri2.vertices[0].m_C = inside_points[1]->m_C;
		outTri2.vertices[1].m_C = outTri1.vertices[2].m_C;
		outTri2.vertices[0].m_UV = inside_points[1]->m_UV;
		outTri2.vertices[1].m_UV = outTri1.vertices[2].m_UV;

		return 2;
	}
}
