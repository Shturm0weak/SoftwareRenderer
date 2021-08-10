#pragma once

#include "Structs.h"

glm::vec3 Normalize(const glm::vec3& vector);
glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n);
glm::vec3 Clamp(const glm::vec3& a, const glm::vec3& b);
glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end, float& t);
int TriangleClipAgainstPlane(glm::vec3 planeP, glm::vec3 planeN, sr::TriangleV& inTri, sr::TriangleV& outTri1, sr::TriangleV& outTri2, bool screenSpace = false);
float Lerp(float y1, float y2, float x);
glm::vec3 Lerp(const glm::vec3& y1, const glm::vec3& y2, float x);
float Max(float a, float b);