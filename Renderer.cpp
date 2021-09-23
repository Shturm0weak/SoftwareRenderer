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

void sr::Renderer::Discard()
{
	Window::GetInstance().m_DiscradPixel = true;
}

void sr::Renderer::FillPixel(const glm::ivec2& pos, const glm::vec4& color, bool IsAlphaBlended)
{
	if (s_DiscradPixel)
	{
		s_DiscradPixel = false;
		return;
	}
    uint8_t* pixel = GetPixel(pos);
    if (pixel == nullptr) return;
	if (IsAlphaBlended)
	{
		glm::vec4 destinationColor = { pixel[2], pixel[1], pixel[0], pixel[3] };
		glm::vec4 blendedColor = AlphaBlend(glm::vec4(color), glm::vec4(destinationColor) / 255.0f);
		pixel[0] = (uint8_t)(blendedColor[2] * 255.0f);
		pixel[1] = (uint8_t)(blendedColor[1] * 255.0f);
		pixel[2] = (uint8_t)(blendedColor[0] * 255.0f);
		pixel[3] = (uint8_t)(blendedColor[3] * 255.0f);
	}
	else
	{
		pixel[0] = (uint8_t)(color[2] * 255.0f);
		pixel[1] = (uint8_t)(color[1] * 255.0f);
		pixel[2] = (uint8_t)(color[0] * 255.0f);
		pixel[3] = (uint8_t)(color[3] * 255.0f);
	}
}

glm::vec4 sr::Renderer::GammaCorrection(const glm::vec4& color, float gamma)
{
	glm::vec4 gammaCorrectedColor = glm::pow(glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f)), glm::vec4(1.0 / gamma));
	gammaCorrectedColor.w = color.w;
	return gammaCorrectedColor;
}

inline glm::vec4 sr::Renderer::AlphaBlend(const glm::vec4& a, const glm::vec4& b)
{
	return glm::vec4((a * a.w + b * b.w * (1 - a.w)) / (a.w + b.w * (1.0f - a.w)));
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
		float scale1 = ((float)(steps - i) / (float)steps);
		float scale2 = 1.0f - scale1;
		glm::ivec4 color = Lerp(v1.m_Color, v2.m_Color, scale2);
        newStart += increment;
        FillPixel(glm::ivec2(glm::round(newStart.x), glm::round(newStart.y)), color);
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
		float t = (float)(v2.m_FragPos.y - v1.m_FragPos.y) / (float)((v3.m_FragPos.y - v1.m_FragPos.y));
		glm::vec2 scale = { t, 1.0f - t };

		Vertex v4;
		v4.m_FragPos = Lerp(v1.m_FragPos, v3.m_FragPos, scale[0]);
		v4.m_Color = Lerp(v1.m_Color, v3.m_Color, scale[0]);
		v4.m_WorldPos = Lerp(v1.m_WorldPos, v3.m_WorldPos, scale[0]);
		v4.m_Normal = Normalize(Lerp(v1.m_Normal, v3.m_Normal, scale[0]));
		v4.m_UV = Lerp(v1.m_UV, v3.m_UV, scale[0]);
		v4.m_Material = v1.m_Material;

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
	Scene& scene = Scene::GetInstance();
	Window& window = Window::GetInstance();
	for (int i = v1.m_FragPos.x; i < v2.m_FragPos.x; i++)
	{
		float t = (float)(v2.m_FragPos.x - i) / (float)(v2.m_FragPos.x - v1.m_FragPos.x);

		glm::ivec2 fragPos = glm::ivec2(glm::round(Lerp(v2.m_FragPos, v1.m_FragPos, t)));
		glm::vec3 worldPos = Lerp(v2.m_WorldPos, v1.m_WorldPos, t);
		glm::vec3 normal = Normalize(Lerp(v2.m_Normal, v1.m_Normal, t));
		glm::vec2 uv = Lerp(v2.m_UV, v1.m_UV, t);
		glm::vec4 color = Lerp(v2.m_Color, v1.m_Color, t);

		//Enabling can fix some flickering pixels due to synchronous writing to the depth buffer, but performance is much worse
		//std::lock_guard lock(window.m_SyncParams.s_ExtraMtx);
		
		float* depthPixel = GetDepthPixel(fragPos);
		if (depthPixel != nullptr)
		{
			float z = Lerp(v2.m_FragPos.z, v1.m_FragPos.z, t);
			uv *= 1.0f / z;
			color *= 1.0f / z;
			z = 1.0f - z;
			if (z < *depthPixel)
			{
				{
					//std::lock_guard lock(window.m_SyncParams.s_ExtraMtx);
					*depthPixel = z;
				}
				switch (window.m_DrawBuffer)
				{
					case BUFFERSTATE::SHADER:
					{
						glm::vec4 fragColor = scene.m_BindedShader->m_FragmentShader(
							worldPos,
							fragPos,
							normal,
							color,
							uv,
							*v1.m_Material
						);
						FillPixel(fragPos, fragColor, v1.m_Material->m_IsAlphaBlended);
					}
					break;
					case BUFFERSTATE::DEPTH:
					{
						FillPixel(fragPos, glm::vec4(z, z, z, 1.0f));
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
		(v3.m_FragPos.x - v1.m_FragPos.x) / (v3.m_FragPos.y - v1.m_FragPos.y),
		(v3.m_FragPos.x - v2.m_FragPos.x) / (v3.m_FragPos.y - v2.m_FragPos.y)
	};

	glm::ivec2 yPos = {
		(int)ceilf(v1.m_FragPos.y - 0.5f),
		(int)ceilf(v3.m_FragPos.y - 0.5f)
	};

	for (int y = yPos[0]; y < yPos[1]; y++)
	{
		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x,
			invSlope[1] * (float(y) + 0.5f - v2.m_FragPos.y) + v2.m_FragPos.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};

		glm::vec2 fragPos[2];
		fragPos[0] = glm::vec2(xPos[0], y);
		fragPos[1] = glm::vec2(xPos[1], y);

		glm::vec2 scale = {
			glm::distance(fragPos[0], glm::vec2(v3.m_FragPos)) / glm::distance(glm::vec2(v1.m_FragPos), glm::vec2(v3.m_FragPos)),
			glm::distance(fragPos[1], glm::vec2(v3.m_FragPos)) / glm::distance(glm::vec2(v2.m_FragPos), glm::vec2(v3.m_FragPos))
		};

		glm::vec4 color[2];
		color[0] = Lerp(v3.m_Color, v1.m_Color, scale[0]);
		color[1] = Lerp(v3.m_Color, v2.m_Color, scale[1]);

		glm::vec2 z = { Lerp(v3.m_FragPos.z, v1.m_FragPos.z, scale[0]), Lerp(v3.m_FragPos.z, v2.m_FragPos.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Lerp(v3.m_WorldPos, v1.m_WorldPos, scale[0]);
		worldPos[1] = Lerp(v3.m_WorldPos, v2.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Normalize(Lerp(v3.m_Normal, v1.m_Normal, scale[0]));
		normal[1] = Normalize(Lerp(v3.m_Normal, v2.m_Normal, scale[1]));

		glm::vec2 uvs[2];
		uvs[0] = Lerp(v3.m_UV, v1.m_UV, scale[0]);
		uvs[1] = Lerp(v3.m_UV, v2.m_UV, scale[1]);

		Vertex vertex1 = { glm::vec3(xPos[0], y, z[0]), worldPos[0], normal[0], color[0], uvs[0], v1.m_Material };
		Vertex vertex2 = { glm::vec3(xPos[1], y, z[1]), worldPos[1], normal[1], color[1], uvs[1], v2.m_Material };

		if (xPos[0] < xPos[1])
		{
			DrawFlatLine(vertex1, vertex2);
		}
		else
		{
			DrawFlatLine(vertex2, vertex1);
		}
	}
}

void sr::Renderer::FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
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
		glm::vec2 px = {
			invSlope[0] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x,
			invSlope[1] * (float(y) + 0.5f - v1.m_FragPos.y) + v1.m_FragPos.x
		};

		glm::ivec2 xPos = {
			(int)ceilf(px[0] - 0.5f),
			(int)ceilf(px[1] - 0.5f)
		};

		glm::vec2 fragPos[2];
		fragPos[0] = glm::vec2(xPos[0], y);
		fragPos[1] = glm::vec2(xPos[1], y);

		glm::vec2 scale = {
			glm::distance(fragPos[0], glm::vec2(v1.m_FragPos)) / glm::distance(glm::vec2(v2.m_FragPos), glm::vec2(v1.m_FragPos)),
			glm::distance(fragPos[1], glm::vec2(v1.m_FragPos)) / glm::distance(glm::vec2(v3.m_FragPos), glm::vec2(v1.m_FragPos))
		};

		glm::vec4 color[2];
		color[0] = Lerp(v1.m_Color, v2.m_Color, scale[0]);
		color[1] = Lerp(v1.m_Color, v3.m_Color, scale[1]);

		glm::vec2 z = { Lerp(v1.m_FragPos.z, v2.m_FragPos.z, scale[0]), Lerp(v1.m_FragPos.z, v3.m_FragPos.z, scale[1]) };

		glm::vec3 worldPos[2];
		worldPos[0] = Lerp(v1.m_WorldPos, v2.m_WorldPos, scale[0]);
		worldPos[1] = Lerp(v1.m_WorldPos, v3.m_WorldPos, scale[1]);

		glm::vec3 normal[2];
		normal[0] = Normalize(Lerp(v1.m_Normal, v2.m_Normal, scale[0]));
		normal[1] = Normalize(Lerp(v1.m_Normal, v3.m_Normal, scale[1]));

		glm::vec2 uvs[2];
		uvs[0] = Lerp(v1.m_UV, v2.m_UV, scale[0]);
		uvs[1] = Lerp(v1.m_UV, v3.m_UV, scale[1]);

		Vertex vertex1 = { glm::vec3(fragPos[0], z[0]), worldPos[0], normal[0], color[0], uvs[0], v1.m_Material };
		Vertex vertex2 = { glm::vec3(fragPos[1], z[1]), worldPos[1], normal[1], color[1], uvs[1], v2.m_Material };

		if (xPos[0] < xPos[1])
		{
			DrawFlatLine(vertex1, vertex2);
		}
		else
		{
			DrawFlatLine(vertex2, vertex1);
		}
	}
}