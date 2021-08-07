#pragma once

#include "Structs.h"

glm::vec3 Normalize(const glm::vec3& vector);
glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n);
glm::vec3 Clamp(const glm::vec3& a, const glm::vec3& b);
glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end);
int Triangle_ClipAgainstPlane(glm::vec3 plane_p, glm::vec3 plane_n, sr::TriangleV& in_tri, sr::TriangleV& out_tri1, sr::TriangleV& out_tri2);
float Interpolate(float y1, float y2, float x);
glm::vec3 Interpolate(const glm::vec3& y1, const glm::vec3& y2, float x);
float Max(float a, float b);