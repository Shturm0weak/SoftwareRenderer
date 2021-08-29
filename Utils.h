#pragma once

#include "Structs.h"

glm::vec3 Normalize(const glm::vec3& vector);
glm::vec3 Reflect(const glm::vec3& i, const glm::vec3& n);
glm::vec3 Clamp(const glm::vec3& a, const glm::vec3& b = glm::vec3(0.0f), const glm::vec3& c = glm::vec3(1.0f));
glm::vec2 Clamp(const glm::vec2& a, const glm::vec2& b = glm::vec2(0.0f), const glm::vec2& c = glm::vec2(1.0f));
glm::vec2 Lerp(const glm::vec2& y1, const glm::vec2& y2, float x);
glm::vec3 Lerp(const glm::vec3& y1, const glm::vec3& y2, float x);
glm::vec4 Lerp(const glm::vec4& y1, const glm::vec4& y2, float x);
glm::ivec3 Lerp(const glm::ivec3& y1, const glm::ivec3& y2, float x);
glm::vec3 IntersectPlane(glm::vec3& planeP, glm::vec3& planeN, glm::vec3& start, glm::vec3& end, float& t);
float Lerp(const float& y1, const float& y2, const float& x);
float Max(float a, float b);
int TriangleClipAgainstPlane(glm::vec3 planeP, glm::vec3 planeN, sr::TriangleV& inTri, sr::TriangleV& outTri1, sr::TriangleV& outTri2, bool screenSpace = false);