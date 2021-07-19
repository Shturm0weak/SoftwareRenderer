#pragma once

namespace sr
{

	class Renderer
	{
	public:

		static uint8_t* GetPixel(glm::ivec2 pos);
		static void FillPixel(glm::ivec2 pos, glm::ivec3 color);
		static void DrawLine(Point v1, Point v2);
		static void FillTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void DrawTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	private:

		static float* GetDepthPixel(glm::ivec2 pos);
		static void DrawFlatLine(Point v1, Point v2);
		static void FillTopFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		static void FillBottomFlatTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	};

}