#include "pch.h"
#include "Utils.h"

inline glm::vec3 Normalize(const glm::vec3& vector)
{
	float l = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return vector * (1.0f / l);
}

inline float Lerp(const float& y1, const float& y2, const float& t)
{
	//return y1 + t * (y2 - y1);
	return (1 - t) * y1 + t * y2;
}

inline glm::vec2 Lerp(const glm::vec2& y1, const glm::vec2& y2, float t)
{
	return { Lerp(y1.x, y2.x, t), Lerp(y1.y, y2.y, t) };
}

inline glm::vec3 Lerp(const glm::vec3& y1, const glm::vec3& y2, float t)
{
	return { Lerp(y1.x, y2.x, t), Lerp(y1.y, y2.y, t), Lerp(y1.z, y2.z, t) };
}

inline glm::vec4 Lerp(const glm::vec4& y1, const glm::vec4& y2, float t)
{
	return { Lerp(y1.x, y2.x, t), Lerp(y1.y, y2.y, t), Lerp(y1.z, y2.z, t), Lerp(y1.w, y2.w, t ) };
}

inline glm::ivec3 Lerp(const glm::ivec3& y1, const glm::ivec3& y2, float t)
{
	return { Lerp(y1.x, y2.x, t), Lerp(y1.y, y2.y, t), Lerp(y1.z, y2.z, t) };
}

float Max(float a, float b)
{
	return a > b ? a : b;
}

glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n)
{
	return i - n * 2.0f * glm::dot(n, i);
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

int TriangleClipAgainstPlane(glm::vec3 planeP, glm::vec3 planeN, sr::Triangle& inTri, sr::Triangle& outTri1, sr::Triangle& outTri2, bool screenSpace)
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
			outTri1.vertices[1].m_FragPos = Lerp(inside_points[0]->m_FragPos, outside_points[0]->m_FragPos, t1);
			outTri1.vertices[2].m_FragPos = Lerp(inside_points[0]->m_FragPos, outside_points[1]->m_FragPos, t2);
		}
		outTri1.vertices[1].m_Normal = Normalize(Lerp(inside_points[0]->m_Normal, outside_points[0]->m_Normal, t1));
		outTri1.vertices[2].m_Normal = Normalize(Lerp(inside_points[0]->m_Normal, outside_points[1]->m_Normal, t2));
		outTri1.vertices[1].m_Color = Lerp(inside_points[0]->m_Color, outside_points[0]->m_Color, t1);
		outTri1.vertices[2].m_Color = Lerp(inside_points[0]->m_Color, outside_points[1]->m_Color, t2);
		outTri1.vertices[1].m_UV = Lerp(inside_points[0]->m_UV, outside_points[0]->m_UV, t1);
		outTri1.vertices[2].m_UV = Lerp(inside_points[0]->m_UV, outside_points[1]->m_UV, t2);

		outTri1.vertices[0] = *inside_points[0];
		
		return 1;
	}
	else if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		float t1, t2;
		outTri1.vertices[2].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[0]->m_WorldPos, outside_points[0]->m_WorldPos, t1);
		outTri2.vertices[2].m_WorldPos = IntersectPlane(planeP, planeN, inside_points[1]->m_WorldPos, outside_points[0]->m_WorldPos, t2);
		if (screenSpace)
		{
			outTri1.vertices[0].m_FragPos = inside_points[0]->m_FragPos;
			outTri1.vertices[1].m_FragPos = inside_points[1]->m_FragPos;
			outTri1.vertices[2].m_FragPos = Lerp(inside_points[0]->m_FragPos, outside_points[0]->m_FragPos, t1);

			outTri2.vertices[0].m_FragPos = inside_points[1]->m_FragPos;
			outTri2.vertices[1].m_FragPos = outTri1.vertices[2].m_FragPos;
			outTri2.vertices[2].m_FragPos = Lerp(inside_points[1]->m_FragPos, outside_points[0]->m_FragPos, t2);
		}
		outTri1.vertices[2].m_Normal = Normalize(Lerp(inside_points[0]->m_Normal, outside_points[0]->m_Normal, t1));
		outTri2.vertices[2].m_Normal = Normalize(Lerp(inside_points[1]->m_Normal, outside_points[0]->m_Normal, t2));
		outTri1.vertices[2].m_Color = Lerp(inside_points[0]->m_Color, outside_points[0]->m_Color, t1);
		outTri2.vertices[2].m_Color = Lerp(inside_points[1]->m_Color, outside_points[0]->m_Color, t2);
		outTri1.vertices[2].m_UV = Lerp(inside_points[0]->m_UV, outside_points[0]->m_UV, t1);
		outTri2.vertices[2].m_UV = Lerp(inside_points[1]->m_UV, outside_points[0]->m_UV, t2);

		outTri1.vertices[0] = *inside_points[0];
		outTri1.vertices[1] = *inside_points[1];

		outTri2.vertices[0] = *inside_points[1];
		outTri2.vertices[1] = outTri1.vertices[2];

		return 2;
	}
}
