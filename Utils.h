#pragma once

#include "Structs.h"

glm::vec3 Normalize(const glm::vec3& vector);
glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n);
float Lerp(const float& y1, const float& y2, const float& t);
glm::vec2 Lerp(const glm::vec2& y1, const glm::vec2& y2, float t);
glm::vec3 Lerp(const glm::vec3& y1, const glm::vec3& y2, float t);
glm::vec4 Lerp(const glm::vec4& y1, const glm::vec4& y2, float t);
glm::ivec3 Lerp(const glm::ivec3& y1, const glm::ivec3& y2, float t);
glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end, float& t);
float Max(float a, float b);
int TriangleClipAgainstPlane(glm::vec3 planeP, glm::vec3 planeN, sr::Triangle& inTri, sr::Triangle& outTri1, sr::Triangle& outTri2, bool screenSpace = false);