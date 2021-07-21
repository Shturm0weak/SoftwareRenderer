#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Scene.h"
#include "Input.h"

uint8_t* sr::Renderer::GetPixel(glm::ivec2 pos)
{
    Window& window = Window::GetInstance();
    if (0 <= pos.x && pos.x < window.s_BitMapSize.x && 0 <= pos.y && pos.y < window.s_BitMapSize.y)
    {
        uint8_t* bitMapMemory = (uint8_t*)window.s_BitMapMemory;
        int position = (pos.x + pos.y * window.s_BitMapSize.x) * 4;
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
		glm::vec3 fragPos = v1.m_P + (v3.m_P - v1.m_P) * alphaSplit;

		Vertex v4;
		v4.m_P = fragPos;

		float scalef = (((v3.m_P.y - v1.m_P.y) - (v4.m_P.y - v1.m_P.y)) / (v3.m_P.y - v1.m_P.y));
		glm::vec2 scale = {
			scalef,
			1.0f - scalef
		};

		v4.m_P.z = Interpolate(v1.m_P.z, v3.m_P.z, scale[1]);

		//the same as v4.m_C = Interpolate(v1.m_C, v3.m_C, scale2);
		v4.m_C = scale[0] * glm::vec3(v1.m_C) + scale[1] * glm::vec3(v3.m_C);
		v4.m_WorldPos = Interpolate(v1.m_WorldPos, v3.m_WorldPos, scale[1]);
		v4.m_Normal = Interpolate(v1.m_Normal, v3.m_Normal, scale[1]);

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
	if (0 <= pos.x && pos.x < window.s_BitMapSize.x && 0 <= pos.y && pos.y < window.s_BitMapSize.y)
	{
		int position = (pos.x + pos.y * window.s_BitMapSize.x);
		return &window.s_DepthBuffer[position];
	}
	return nullptr;
}

void sr::Renderer::DrawFlatLine(Point v1, Point v2)
{
	Scene& scene = Scene::GetInstance();
	Window& window = Window::GetInstance();
	float d = v2.m_P.x - v1.m_P.x;
	int steps = glm::abs(d);
	if (steps == 0) return;
	float increment = d / (float)steps;
	glm::vec2 newStart = v1.m_P;
	for (int i = 0; i < steps; i++)
	{
		newStart.x += increment;

		float scalef = ((steps - i) / (float)steps);
		glm::vec2 scale = {
			scalef,
			1.0f - scalef
		};

		glm::ivec3 color = scale[0] * glm::vec3(v1.m_C) + scale[1] * glm::vec3(v2.m_C);
		glm::ivec2 fragPos = glm::ivec2(glm::ceil(newStart.x), glm::ceil(newStart.y));

		glm::vec3 worldPos = Interpolate(v2.m_WorldPos, v1.m_WorldPos, scale[0]);
		glm::vec3 normal = Interpolate(v2.m_Normal, v1.m_Normal, scale[0]);

		float* depthPixel = GetDepthPixel(fragPos);
		if (depthPixel != nullptr)
		{
			float z = 1.0f - (1.0f / Interpolate(v2.m_Z, v1.m_Z, scale[0]));
			if (z < *depthPixel)
			{
				*depthPixel = z;
				switch (window.s_DrawBuffer)
				{
					case BUFFER_STATE::AMBIENT:
					{
						FillPixel(fragPos, color);
					}
					break;
					case BUFFER_STATE::SHADER:
					{
						glm::ivec3 fragColor = scene.s_BindedShader->m_FragmentShader(worldPos, fragPos, normal, color);
						FillPixel(fragPos, fragColor);
					}
					break;
					case BUFFER_STATE::DEPTH:
					{
						FillPixel(fragPos, glm::ivec3((1.0f - glm::abs(z)) * 255.0f));
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
	glm::vec2 invSlope = {
		(v3.m_P.x - v1.m_P.x) / (v3.m_P.y - v1.m_P.y),
		(v3.m_P.x - v2.m_P.x) / (v3.m_P.y - v2.m_P.y)
	};

	glm::ivec2 yPos = {
		(int)ceilf(v1.m_P.y - 0.5f),
		(int)ceilf(v3.m_P.y - 0.5f)
	};

	for (int y = yPos[0]; y < yPos[1]; y++)
	{
		float scalef = (float)(yPos[1] - y) / (yPos[1] - yPos[0]);
		glm::vec2 scale = {
			scalef,
			1.0f - scalef
		};

		glm::ivec3 color[2];
		color[0] = scale[0] * glm::vec3(v1.m_C) + scale[1] * glm::vec3(v3.m_C);
		color[1] = scale[0] * glm::vec3(v2.m_C) + scale[1] * glm::vec3(v3.m_C);

		glm::vec2 z = { Interpolate(v1.m_P.z, v3.m_P.z, scale[1]), Interpolate(v2.m_P.z, v3.m_P.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Interpolate(v1.m_WorldPos, v3.m_WorldPos, scale[1]);
		worldPos[1] = Interpolate(v2.m_WorldPos, v3.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Interpolate(v1.m_Normal, v3.m_Normal, scale[1]);
		normal[1] = Interpolate(v2.m_Normal, v3.m_Normal, scale[1]);

		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x,
			invSlope[1] * (float(y) + 0.5f - v2.m_P.y) + v2.m_P.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};

		Point p1(glm::ivec2(xPos[0], y), color[0], worldPos[0], normal[0], z[0]);
		Point p2(glm::ivec2(xPos[1], y), color[1], worldPos[1], normal[1], z[0]);

		DrawFlatLine(p1, p2);
	}
}

void sr::Renderer::FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	glm::vec2 invSlope = {
		(v2.m_P.x - v1.m_P.x) / (v2.m_P.y - v1.m_P.y),
		(v3.m_P.x - v1.m_P.x) / (v3.m_P.y - v1.m_P.y)
	};

	glm::ivec2 yPos = {
		(int)ceilf(v1.m_P.y - 0.5f),
		(int)ceilf(v3.m_P.y - 0.5f)
	};

	for (int y = yPos[0]; y < yPos[1]; y++)
	{
		float scalef = (float)(yPos[1] - y) / (yPos[1] - yPos[0]);
		glm::vec2 scale = {
			scalef,
			1.0f - scalef
		};

		glm::ivec3 color[2];
		color[0] = scale[0] * glm::vec3(v1.m_C) + scale[1] * glm::vec3(v2.m_C);
		color[1] = scale[0] * glm::vec3(v1.m_C) + scale[1] * glm::vec3(v3.m_C);

		glm::vec2 z = { Interpolate(v1.m_P.z, v2.m_P.z, scale[1]), Interpolate(v1.m_P.z, v3.m_P.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Interpolate(v1.m_WorldPos, v2.m_WorldPos, scale[1]);
		worldPos[1] = Interpolate(v1.m_WorldPos, v3.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Interpolate(v1.m_Normal, v2.m_Normal, scale[1]);
		normal[1] = Interpolate(v1.m_Normal, v3.m_Normal, scale[1]);

		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x,
			invSlope[1] * (float(y) + 0.5f - v1.m_P.y) + v1.m_P.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};
		
		Point p1(glm::ivec2(xPos[0], y), color[0], worldPos[0], normal[0], z[0]);
		Point p2(glm::ivec2(xPos[1], y), color[1], worldPos[1], normal[1], z[1]);

		DrawFlatLine(p1, p2);
	}
}