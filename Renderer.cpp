#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Scene.h"
#include "Input.h"

uint8_t* sr::Renderer::GetPixel(const glm::ivec2& pos)
{
    Window& window = Window::GetInstance();
    if (0 <= pos.x && pos.x < window.m_BitMapSize.x && 0 <= pos.y && pos.y < window.m_BitMapSize.y)
	{
        int position = (pos.x + pos.y * window.m_BitMapSize.x) * 4;
        return &((uint8_t*)window.m_BitMapMemory)[position];
    }
    return nullptr;
}

void sr::Renderer::FillPixel(const glm::ivec2& pos, const glm::ivec3& color)
{
    uint8_t* pixel = GetPixel(pos);
    if (pixel == nullptr) return;
    pixel[0] = color.z;
    pixel[1] = color.y;
    pixel[2] = color.x;
}

void sr::Renderer::DrawLine(Vertex& v1, Vertex& v2)
{
    glm::vec2 d = { (v2.m_FragPos.x - v1.m_FragPos.x), (v2.m_FragPos.y - v1.m_FragPos.y) };
    int steps = glm::abs(d.x) > glm::abs(d.y) ? glm::abs(d.x) : glm::abs(d.y);
	if (steps == 0) return;
	glm::vec2 increment = { d.x / (float)steps, d.y / (float)steps };

    glm::vec2 newStart = v1.m_FragPos;
    for (int i = 0; i < steps; i++)
    {
		float scale1 = ((steps - i) / (float)steps);
		float scale2 = 1.0f - scale1;
		glm::ivec3 c = scale1 * glm::vec3(v1.m_Color) + scale2 * glm::vec3(v2.m_Color);
        newStart += increment;
        FillPixel(glm::ivec2(glm::ceil(newStart.x), glm::ceil(newStart.y)), c);
    }
}

void sr::Renderer::FillTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	if (v2.m_FragPos.y < v1.m_FragPos.y) std::swap<Vertex>(v1, v2);
	if (v3.m_FragPos.y < v2.m_FragPos.y) std::swap<Vertex>(v2, v3);
	if (v2.m_FragPos.y < v1.m_FragPos.y) std::swap<Vertex>(v1, v2);

	if (v2.m_FragPos.y == v3.m_FragPos.y)
	{
		if (v3.m_FragPos.x < v2.m_FragPos.x) std::swap<Vertex>(v2, v3);
		FillBottomFlatTriangle(v1, v3, v2);
	}
	else if (v1.m_FragPos.y == v2.m_FragPos.y)
	{
		if (v2.m_FragPos.x < v1.m_FragPos.x) std::swap<Vertex>(v1, v2);
		FillTopFlatTriangle(v1, v2, v3);
	}
	else
	{
		//float t = ((v3.m_FragPos.y - v1.m_FragPos.y) - (v4.m_FragPos.y - v1.m_FragPos.y)) / (v3.m_FragPos.y - v1.m_FragPos.y);
		float t = (float)(v2.m_FragPos.y - v1.m_FragPos.y) / (float)(v3.m_FragPos.y - v1.m_FragPos.y);
		glm::vec2 scale = { 1.0f - t, t };

		Vertex v4;
		v4.m_FragPos = Lerp(v1.m_FragPos, v3.m_FragPos, scale[1]);
		v4.m_Color = Lerp(v1.m_Color, v3.m_Color, scale[1]);
		v4.m_WorldPos = Lerp(v1.m_WorldPos, v3.m_WorldPos, scale[1]);
		v4.m_Normal = Lerp(v1.m_Normal, v3.m_Normal, scale[1]);
		v4.m_UV = Lerp(v1.m_UV, v3.m_UV, scale[1]);
		v4.m_Material = v2.m_Material;

		if (v2.m_FragPos.x < v4.m_FragPos.x)
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


float* sr::Renderer::GetDepthPixel(const glm::ivec2& pos)
{
	Window& window = Window::GetInstance();
	if (0 <= pos.x && pos.x < window.m_BitMapSize.x && 0 <= pos.y && pos.y < window.m_BitMapSize.y)
	{
		int position = (pos.x + pos.y * window.m_BitMapSize.x);
		return &window.m_DepthBuffer[position];
	}
	return nullptr;
}

void sr::Renderer::DrawFlatLine(Vertex& v1, Vertex& v2)
{
	//The dumbest way to get rid of gaps between triangles when clip against the top side of the screen
	//Disabling can fix some flickering on edges (not sure)
	if (v1.m_FragPos.x < v2.m_FragPos.x) v1.m_FragPos.x--;

	Scene& scene = Scene::GetInstance();
	Window& window = Window::GetInstance();
	float distance = v2.m_FragPos.x - v1.m_FragPos.x;
	int steps = glm::abs(distance);
	if (steps == 0) return;
	float increment = distance / (float)steps;
	glm::vec2 newStart = v1.m_FragPos;
	for (int i = 0; i < steps; i++)
	{
		newStart.x += increment;

		float t = float(steps - i) / float(steps);
		glm::vec2 scale = { t, 1.0f - t };

		glm::ivec2 fragPos = glm::ivec2(newStart);
		glm::vec3 worldPos = Lerp(v2.m_WorldPos, v1.m_WorldPos, scale[0]);
		glm::vec3 normal = Lerp(v2.m_Normal, v1.m_Normal, scale[0]);
		glm::vec2 uv = Lerp(v2.m_UV, v1.m_UV, scale[0]);
		glm::vec3 color = Lerp(v2.m_Color, v1.m_Color, scale[0]);
		color *= (1.0f / 255.0f);
		//Enabling can fix some flickering pixels due to synchronous writing to the depth buffer, but performance is much worse
		//std::lock_guard lock(window.m_SyncParams.s_Mtx);
		
		float* depthPixel = GetDepthPixel(fragPos);
		if (depthPixel != nullptr)
		{
			float z = Lerp(v2.m_FragPos.z, v1.m_FragPos.z, scale[0]);
			uv *= 1.0 / z;
			z = 1.0f - z;
			if (z < *depthPixel)
			{
				*depthPixel = z;
				switch (window.m_DrawBuffer)
				{
					case BUFFERSTATE::SHADER:
					{
						glm::ivec3 fragColor = scene.m_BindedShader->m_FragmentShader(
							worldPos,
							fragPos,
							normal,
							color,
							uv,
							*v1.m_Material
						) * 255.0f;
						FillPixel(fragPos, fragColor);
					}
					break;
					case BUFFERSTATE::DEPTH:
					{
						FillPixel(fragPos, glm::ivec3((1.0f - sqrtf(glm::abs(z))) * 255.0f));
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
	v1.m_Color = glm::ivec3(150, 0, 0);
	v2.m_Color = glm::ivec3(150, 0, 0);
	v3.m_Color = glm::ivec3(150, 0, 0);
	
	glm::vec2 invSlope = {
		(v3.m_FragPos.x - v1.m_FragPos.x) / (v3.m_FragPos.y - v1.m_FragPos.y),
		(v3.m_FragPos.x - v2.m_FragPos.x) / (v3.m_FragPos.y - v2.m_FragPos.y)
	};

	glm::ivec2 yPos = {
		(int)ceilf(v1.m_FragPos.y - 0.5f),
		(int)ceilf(v3.m_FragPos.y - 0.5f)
	};

	for (int y = yPos[0]; y < yPos[1]; y++)
	{
		float t = (float)(yPos[1] - y) / (float)(yPos[1] - yPos[0]);
		glm::vec2 scale = { t, 1.0f - t };

		glm::ivec3 color[2];
		color[0] = Lerp(v1.m_Color, v3.m_Color, scale[1]);
		color[1] = Lerp(v2.m_Color, v3.m_Color, scale[1]);

		glm::vec2 z = { Lerp(v1.m_FragPos.z, v3.m_FragPos.z, scale[1]), Lerp(v2.m_FragPos.z, v3.m_FragPos.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Lerp(v1.m_WorldPos, v3.m_WorldPos, scale[1]);
		worldPos[1] = Lerp(v2.m_WorldPos, v3.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Lerp(v1.m_Normal, v3.m_Normal, scale[1]);
		normal[1] = Lerp(v2.m_Normal, v3.m_Normal, scale[1]);

		glm::vec2 uvs[2];
		uvs[0] = Lerp(v1.m_UV, v3.m_UV, scale[1]);
		uvs[1] = Lerp(v2.m_UV, v3.m_UV, scale[1]);

		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x,
			invSlope[1] * (float(y) + 0.5f - v2.m_FragPos.y) + v2.m_FragPos.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};

		Vertex vertex1 = { glm::vec3(xPos[0], y, z[0]), worldPos[0], normal[0], color[0], uvs[0], v1.m_Material };
		Vertex vertex2 = { glm::vec3(xPos[1], y, z[1]), worldPos[1], normal[1], color[1], uvs[1], v2.m_Material };

		DrawFlatLine(vertex1, vertex2);
	}
}

void sr::Renderer::FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	v1.m_Color = glm::ivec3(0, 0, 150);
	v2.m_Color = glm::ivec3(0, 0, 150);
	v3.m_Color = glm::ivec3(0, 0, 150);

	glm::vec2 invSlope = {
		(v2.m_FragPos.x - v1.m_FragPos.x) / (v2.m_FragPos.y - v1.m_FragPos.y),
		(v3.m_FragPos.x - v1.m_FragPos.x) / (v3.m_FragPos.y - v1.m_FragPos.y)
	};

	glm::ivec2 yPos = {
		(int)ceilf(v1.m_FragPos.y - 0.5f),
		(int)ceilf(v3.m_FragPos.y - 0.5f)
	};

	for (int y = yPos[0]; y < yPos[1]; y++)
	{
 		float t = (float)(yPos[1] - y) / (float)(yPos[1] - yPos[0]);
		glm::vec2 scale = { t, 1.0f - t };

		glm::ivec3 color[2];
		color[0] = Lerp(v1.m_Color, v2.m_Color, scale[1]);
		color[1] = Lerp(v1.m_Color, v3.m_Color, scale[1]);

		glm::vec2 z = { Lerp(v1.m_FragPos.z, v2.m_FragPos.z, scale[1]), Lerp(v1.m_FragPos.z, v3.m_FragPos.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Lerp(v1.m_WorldPos, v2.m_WorldPos, scale[1]);
		worldPos[1] = Lerp(v1.m_WorldPos, v3.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Lerp(v1.m_Normal, v2.m_Normal, scale[1]);
		normal[1] = Lerp(v1.m_Normal, v3.m_Normal, scale[1]);

		glm::vec2 uvs[2];
		uvs[0] = Lerp(v1.m_UV, v2.m_UV, scale[1]);
		uvs[1] = Lerp(v1.m_UV, v3.m_UV, scale[1]);

		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x,
			invSlope[1] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};
		
		Vertex vertex1 = { glm::vec3(xPos[0], y, z[0]), worldPos[0], normal[0], color[0], uvs[0], v1.m_Material };
		Vertex vertex2 = { glm::vec3(xPos[1], y, z[1]), worldPos[1], normal[1], color[1], uvs[1], v2.m_Material };

		DrawFlatLine(vertex1, vertex2);
	}
}