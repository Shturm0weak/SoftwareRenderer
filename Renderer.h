#pragma once

namespace sr
{

	class Renderer
	{
	public:

		static uint8_t* GetPixel(const glm::ivec2& pos);
		static void FillPixel(const glm::ivec2& pos, const glm::ivec4& color);
		static void DrawLine(Vertex& v1, Vertex& v2);
		static void FillTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void DrawTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void Discard();
		static glm::vec4 GammaCorrection(const glm::vec4& color, float gamma);
	private:

		static glm::vec4 AlphaBlend(const glm::vec4& a, const glm::vec4& b);
		static float* GetDepthPixel(const glm::ivec2& pos);
		static void DrawFlatLine(Vertex& v1, Vertex& v2);
		static void FillTopFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	};

}