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

int Triangle_ClipAgainstPlane(glm::vec3 plane_p, glm::vec3 plane_n, sr::TriangleV& in_tri, sr::TriangleV& out_tri1, sr::TriangleV& out_tri2)
{
	// Make sure plane normal is indeed normal
	plane_n = Normalize(plane_n);

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](glm::vec3& p)
	{
		glm::vec3 n = Normalize(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - glm::dot(plane_n, plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	glm::vec3* inside_points[3];  int nInsidePointCount = 0;
	glm::vec3* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(in_tri.vertices[0].m_WorldPos);
	float d1 = dist(in_tri.vertices[1].m_WorldPos);
	float d2 = dist(in_tri.vertices[2].m_WorldPos);

	if (d0 >= 0)
	{
		inside_points[nInsidePointCount++] = &in_tri.vertices[0].m_WorldPos;
	}
	else 
	{ 
		outside_points[nOutsidePointCount++] = &in_tri.vertices[0].m_WorldPos;
	}
	if (d1 >= 0)
	{
		inside_points[nInsidePointCount++] = &in_tri.vertices[1].m_WorldPos;
	}
	else
	{
		outside_points[nOutsidePointCount++] = &in_tri.vertices[1].m_WorldPos;
	}
	if (d2 >= 0)
	{ 
		inside_points[nInsidePointCount++] = &in_tri.vertices[2].m_WorldPos;
	}
	else
	{ 
		outside_points[nOutsidePointCount++] = &in_tri.vertices[2].m_WorldPos;
	}

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		out_tri1 = in_tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// Copy appearance info to new triangle
		//out_tri1.col = in_tri.col;
		//out_tri1.sym = in_tri.sym;

		// The inside point is valid, so keep that...
		out_tri1.vertices[0].m_WorldPos = *inside_points[0];
		out_tri1.vertices[0].m_Normal = in_tri.vertices[0].m_Normal;
		out_tri1.vertices[0].m_C = in_tri.vertices[0].m_C;
		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		out_tri1.vertices[1].m_WorldPos = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_tri1.vertices[2].m_WorldPos = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);
		out_tri1.vertices[1].m_Normal = in_tri.vertices[1].m_Normal;
		out_tri1.vertices[2].m_Normal = in_tri.vertices[2].m_Normal;
		out_tri1.vertices[1].m_C = in_tri.vertices[1].m_C;
		out_tri1.vertices[2].m_C = in_tri.vertices[2].m_C;

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped triangle becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		//out_tri1.col = in_tri.col;
		//out_tri1.sym = in_tri.sym;

		//out_tri2.col = in_tri.col;
		//out_tri2.sym = in_tri.sym;

		// The first triangle consists of the two inside points and a new
		// point determined by the location where one side of the triangle
		// intersects with the plane
		out_tri1.vertices[0].m_WorldPos = *inside_points[0];
		out_tri1.vertices[1].m_WorldPos = *inside_points[1];
		out_tri1.vertices[2].m_WorldPos = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_tri1.vertices[0].m_Normal = in_tri.vertices[0].m_Normal;
		out_tri1.vertices[1].m_Normal = in_tri.vertices[1].m_Normal;
		out_tri1.vertices[2].m_Normal = in_tri.vertices[2].m_Normal;
		out_tri1.vertices[0].m_C = in_tri.vertices[0].m_C;
		out_tri1.vertices[1].m_C = in_tri.vertices[1].m_C;
		out_tri1.vertices[2].m_C = in_tri.vertices[2].m_C;

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		out_tri2.vertices[0].m_WorldPos = *inside_points[1];
		out_tri2.vertices[1].m_WorldPos = out_tri1.vertices[2].m_WorldPos;
		out_tri2.vertices[2].m_WorldPos = IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);
		out_tri2.vertices[0].m_Normal = in_tri.vertices[0].m_Normal;
		out_tri2.vertices[1].m_Normal = in_tri.vertices[1].m_Normal;
		out_tri2.vertices[2].m_Normal = in_tri.vertices[2].m_Normal;
		out_tri2.vertices[0].m_C = in_tri.vertices[0].m_C;
		out_tri2.vertices[1].m_C = in_tri.vertices[1].m_C;
		out_tri2.vertices[2].m_C = in_tri.vertices[2].m_C;

		return 2; // Return two newly formed triangles which form a quad
	}
}
