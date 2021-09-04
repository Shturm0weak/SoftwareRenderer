#pragma once

namespace sr
{

	class Renderer
	{
	public:

		static uint8_t* GetPixel(const glm::ivec2& pos);
		static void FillPixel(const glm::ivec2& pos, const glm::ivec3& color);
		static void DrawLine(Vertex& v1, Vertex& v2);
		static void FillTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void DrawTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	private:

		static float* GetDepthPixel(const glm::ivec2& pos);
		static void DrawFlatLine(Vertex& v1, Vertex& v2);
		static void FillTopFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	};

}