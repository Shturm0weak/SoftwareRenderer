#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Scene.h"
#include "Input.h"

uint8_t* sr::Renderer::GetPixel(glm::ivec2 pos)
{
    Window& window = Window::GetInstance();
    if (0 <= pos.x && pos.x < window.m_BitMapSize.x && 0 <= pos.y && pos.y < window.m_BitMapSize.y)
    {
        uint8_t* bitMapMemory = (uint8_t*)window.m_BitMapMemory;
        int position = (pos.x + pos.y * window.m_BitMapSize.x) * 4;
        return &bitMapMemory[position];
    }
    return nullptr;
}

void sr::Renderer::FillPixel(glm::ivec2 pos, glm::ivec3 color)
{
    uint8_t* pixel = GetPixel(pos);
    if (pixel == nullptr) return;
    pixel[0] = color.z;
    pixel[1] = color.y;
    pixel[2] = color.x;
}

void sr::Renderer::DrawLine(Point v1, Point v2)
{
    glm::vec2 d = { (v2.m_P.x - v1.m_P.x), (v2.m_P.y - v1.m_P.y) };
    int steps = glm::abs(d.x) > glm::abs(d.y) ? glm::abs(d.x) : glm::abs(d.y);
	if (steps == 0) return;
	glm::vec2 increment = { d.x / (float)steps, d.y / (float)steps };

    glm::vec2 newStart = v1.m_P;
    for (int i = 0; i < steps; i++)
    {
		float scale1 = ((steps - i) / (float)steps);
		float scale2 = 1.0f - scale1;
		glm::ivec3 c = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v2.m_C);
        newStart += increment;
        FillPixel(glm::ivec2(glm::ceil(newStart.x), glm::ceil(newStart.y)), c);
    }
}

void sr::Renderer::FillTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	if (v2.m_P.y < v1.m_P.y) std::swap<Vertex>(v1, v2);
	if (v3.m_P.y < v2.m_P.y) std::swap<Vertex>(v2, v3);
	if (v2.m_P.y < v1.m_P.y) std::swap<Vertex>(v1, v2);

	if (v2.m_P.y == v3.m_P.y)
	{
		if (v3.m_P.x < v2.m_P.x) std::swap<Vertex>(v2, v3);
		FillBottomFlatTriangle(v1, v3, v2);
	}
	else if (v1.m_P.y == v2.m_P.y)
	{
		if (v2.m_P.x < v1.m_P.x) std::swap<Vertex>(v1, v2);
		FillTopFlatTriangle(v1, v2, v3);
	}
	else
	{
		float alphaSplit = ((float)(v2.m_P.y - v1.m_P.y) / (float)(v3.m_P.y - v1.m_P.y));
		glm::vec3 v4pos = v1.m_P + (v3.m_P - v1.m_P) * alphaSplit;

		Vertex v4;
		v4.m_P = v4pos;

		float scale1 = (((v3.m_P.y - v1.m_P.y) - (v4.m_P.y - v1.m_P.y)) / (v3.m_P.y - v1.m_P.y));
		float scale2 = 1.0f - scale1;// ((v4.m_P.y - v1.m_P.y) / (v3.m_P.y - v1.m_P.y));

		v4.m_C = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v3.m_C);
		v4.m_P.z = InterpolateBetweenTwoDots(v1.m_P.z, v3.m_P.z, scale2);

		//std::cout << v1.m_P.z << " " << v3.m_P.z << " " << v4.m_P.z << "\n";

		if (v2.m_P.x < v4.m_P.x)
		{
			FillBottomFlatTriangle(v1, v2, v4);
			FillTopFlatTriangle(v2, v4, v3);
		}
		else
		{
			FillBottomFlatTriangle(v1, v4, v2);
			FillTopFlatTriangle(v4, v2, v3);
		}
	}
}

void sr::Renderer::DrawTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	DrawLine(v1, v2);
	DrawLine(v1, v3);
	DrawLine(v2, v3);
}


float* sr::Renderer::GetDepthPixel(glm::ivec2 pos)
{
	Window& window = Window::GetInstance();
	if (0 <= pos.x && pos.x < window.m_BitMapSize.x && 0 <= pos.y && pos.y < window.m_BitMapSize.y)
	{
		int position = (pos.x + pos.y * window.m_BitMapSize.x);
		return &window.m_DepthBuffer[position];
	}
	return nullptr;
}

void sr::Renderer::DrawFlatLine(Point v1, Point v2)
{
	float d = v2.m_P.x - v1.m_P.x;
	int steps = glm::abs(d);
	if (steps == 0) return;
	float increment = d / (float)steps;
	Scene& scene = Scene::GetInstance();
	Window& window = Window::GetInstance();
	glm::vec2 newStart = v1.m_P;
	for (int i = 0; i < steps; i++)
	{
		float scale1 = ((steps - i) / (float)steps);
		float scale2 = 1.0f - scale1;
		glm::ivec3 c = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v2.m_C);
		newStart.x += increment;
		
		glm::ivec2 fragPos = glm::ivec2(glm::ceil(newStart.x), glm::ceil(newStart.y));
		float z = 1.0f - (1.0f / InterpolateBetweenTwoDots(v2.m_Z, v1.m_Z, scale1));
		float* depthPixel = GetDepthPixel(fragPos);
		if (depthPixel != nullptr)
		{
			if (z < *depthPixel)
			{
				*depthPixel = z;
				switch (window.m_DrawBuffer)
				{
					case BUFFER_STATE::AMBIENT:
					{
						FillPixel(fragPos, c);
					}
					break;
					case BUFFER_STATE::SHADER:
					{
						glm::ivec3 fragColor = scene.m_BindedShader->m_FragmentShader(fragPos, scene.m_OutParams.m_Normal, c);
						FillPixel(fragPos, fragColor);
					}
					break;
					case BUFFER_STATE::DEPTH:
					{
						FillPixel(fragPos, glm::ivec3(z * 255));
					}
					break;
				}	
			}
			else continue;
		}
	}
}

void sr::Renderer::FillTopFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	float invslope1 = (v3.m_P.x - v1.m_P.x) / (v3.m_P.y - v1.m_P.y);
	float invslope2 = (v3.m_P.x - v2.m_P.x) / (v3.m_P.y - v2.m_P.y);

	int yStart = (int)ceilf(v1.m_P.y - 0.5f);
	int yEnd = (int)ceilf(v3.m_P.y - 0.5f);

	for (int y = yStart; y < yEnd; y++)
	{
		float scale1 = (float)(yEnd - y) / (yEnd - yStart);
		float scale2 = 1.0f - scale1;// (float)(y - yStart) / yEnd;
		glm::ivec3 c1 = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v3.m_C);
		glm::ivec3 c2 = scale1 * glm::vec3(v2.m_C) + scale2 * glm::vec3(v3.m_C);

		float z1 = InterpolateBetweenTwoDots(v1.m_P.z, v3.m_P.z, scale2);
		float z2 = InterpolateBetweenTwoDots(v2.m_P.z, v3.m_P.z, scale2);

		float px1 = invslope1 * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x;
		float px2 = invslope2 * (float(y) + 0.5f - v2.m_P.y) + v2.m_P.x;

		int xStart = (int)ceilf(px1 - 0.5f);
		int xEnd = (int)ceilf(px2 - 0.5f);

		Point p1(glm::ivec2(xStart, y), c1, z1);
		Point p2(glm::ivec2(xEnd, y), c2, z2);

		DrawFlatLine(p1, p2);
	}
}

void sr::Renderer::FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	float invslope1 = (v2.m_P.x - v1.m_P.x) / (v2.m_P.y - v1.m_P.y);
	float invslope2 = (v3.m_P.x - v1.m_P.x) / (v3.m_P.y - v1.m_P.y);

	int yStart = (int)ceilf(v1.m_P.y - 0.5f);
	int yEnd = (int)ceilf(v3.m_P.y - 0.5f);

	for (int y = yStart; y < yEnd; y++)
	{
		float scale1 = (float)(yEnd - y) / (yEnd - yStart);
		float scale2 = 1.0f - scale1;// (float)(y - yStart) / yEnd;
		glm::ivec3 c1 = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v2.m_C);
		glm::ivec3 c2 = scale1 * glm::vec3(v1.m_C) + scale2 * glm::vec3(v3.m_C);

		float z1 = InterpolateBetweenTwoDots(v1.m_P.z, v2.m_P.z, scale2);
		float z2 = InterpolateBetweenTwoDots(v1.m_P.z, v3.m_P.z, scale2);

		float px1 = invslope1 * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x;
		float px2 = invslope2 * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x;

		int xStart = (int)ceilf(px1 - 0.5f);
		int xEnd = (int)ceilf(px2 - 0.5f);
		
		Point p1(glm::ivec2(xStart, y), c1, z1);
		Point p2(glm::ivec2(xEnd, y), c2, z2);

		DrawFlatLine(p1, p2);
	}
}