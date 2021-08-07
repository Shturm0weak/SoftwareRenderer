#include "pch.h"
#include "Scene.h"
#include "Window.h"
#include "Renderer.h"

sr::GameObject::GameObject()
{
	Scene::GetInstance().s_GameObjects.push_back(this);
}

inline sr::Scene& sr::Scene::GetInstance()
{
    static Scene scene;
    return scene;
}

void sr::Scene::DrawGameObjects()
{
	Window& window = Window::GetInstance();
	Scene& scene = Scene::GetInstance();
	std::vector<TriangleV> trianglesToRaster;
	for (GameObject* go : s_GameObjects)
	{
		if (go->m_Mesh == nullptr) continue;
		for (Triangle& triangle : go->m_Mesh->m_Triangles)
		{
			glm::vec3 points[3];
			points[0] = go->m_Mesh->m_Vertices[triangle.m_Indices[0]];
			points[1] = go->m_Mesh->m_Vertices[triangle.m_Indices[1]];
			points[2] = go->m_Mesh->m_Vertices[triangle.m_Indices[2]];

			glm::vec3 normals[3];
			normals[0] = go->m_Mesh->m_Normals[triangle.m_Indices[0]];
			normals[1] = go->m_Mesh->m_Normals[triangle.m_Indices[1]];
			normals[2] = go->m_Mesh->m_Normals[triangle.m_Indices[2]];

			glm::vec3 transformedNormals[3];
			transformedNormals[0] = go->m_Transform.m_View * glm::vec4(normals[0], 1.0f);
			transformedNormals[1] = go->m_Transform.m_View * glm::vec4(normals[1], 1.0f);
			transformedNormals[2] = go->m_Transform.m_View * glm::vec4(normals[2], 1.0f);

			glm::mat4 transform = go->m_Transform.m_Model * go->m_Transform.m_View * go->m_Transform.m_Scale;

			glm::vec4 transformedPoints[3];
			transformedPoints[0] = transform * glm::vec4(points[0], 1.0f);
			transformedPoints[1] = transform * glm::vec4(points[1], 1.0f);
			transformedPoints[2] = transform * glm::vec4(points[2], 1.0f);

			glm::vec3 averageNormal = {
				(transformedNormals[0].x + transformedNormals[1].x + transformedNormals[2].x) / 3.0f,
				(transformedNormals[0].y + transformedNormals[1].y + transformedNormals[2].y) / 3.0f,
				(transformedNormals[0].z + transformedNormals[1].z + transformedNormals[2].z) / 3.0f,
			};

			glm::vec3 averageTransformedPoint = {
				(transformedPoints[0].x + transformedPoints[1].x + transformedPoints[2].x) / 3.0f,
				(transformedPoints[0].y + transformedPoints[1].y + transformedPoints[2].y) / 3.0f,
				(transformedPoints[0].z + transformedPoints[1].z + transformedPoints[2].z) / 3.0f,
			};

			bool cullface = glm::dot(averageNormal, averageTransformedPoint - scene.s_Camera.m_Position) < 0.0f;
			if (cullface)
			{
				TriangleV triangleToClip;
				triangleToClip.vertices[0].m_WorldPos = transformedPoints[0];
				triangleToClip.vertices[0].m_C = triangle.m_C[0];
				triangleToClip.vertices[0].m_Normal = transformedNormals[0];

				triangleToClip.vertices[1].m_WorldPos = transformedPoints[1];
				triangleToClip.vertices[1].m_C = triangle.m_C[1];
				triangleToClip.vertices[1].m_Normal = transformedNormals[1];

				triangleToClip.vertices[2].m_WorldPos = transformedPoints[2];
				triangleToClip.vertices[2].m_C = triangle.m_C[2];
				triangleToClip.vertices[2].m_Normal = transformedNormals[2];
				
				int clippedTriangles = 0;
				TriangleV clipped[2];
				clippedTriangles = Triangle_ClipAgainstPlane(
					(scene.s_Camera.m_Forward * 0.7f) + scene.s_Camera.m_Position,
					scene.s_Camera.m_Forward,
					triangleToClip,
					clipped[0],
					clipped[1]);

				for (int n = 0; n < clippedTriangles; n++)
				{

					glm::vec3 p[3];
					for (size_t i = 0; i < 3; i++)
					{
						glm::vec4 fragPos = scene.s_BindedShader->m_VertexShader(
							go->m_Transform,
							transform,
							glm::vec4(clipped[n].vertices[i].m_WorldPos, 1.0f),
							points[i],
							clipped[n].vertices[i].m_Normal);

						p[i] = fragPos;
						float w = fragPos.w;
						if (w != 0.0f)
						{
							p[i] *= 1.0f / w;
						}
					}

					p[0].x += 1.0f;
					p[1].x += 1.0f;
					p[2].x += 1.0f;

					p[0].y += 1.0f;
					p[1].y += 1.0f;
					p[2].y += 1.0f;

					p[0].x *= 0.5f * window.s_Size.x;
					p[1].x *= 0.5f * window.s_Size.x;
					p[2].x *= 0.5f * window.s_Size.x;

					p[0].y *= 0.5f * window.s_Size.y;
					p[1].y *= 0.5f * window.s_Size.y;
					p[2].y *= 0.5f * window.s_Size.y;

					clipped[n].vertices[0].m_P = p[0];
					clipped[n].vertices[1].m_P = p[1];
					clipped[n].vertices[2].m_P = p[2];

					trianglesToRaster.push_back(clipped[n]);
					//Renderer::FillTriangle(clipped[n].vertices[0], clipped[n].vertices[1], clipped[n].vertices[2]);
					//Renderer::DrawTriangle(clipped[n].vertices[0], clipped[n].vertices[1], clipped[n].vertices[2]);
				}
			}
		}
	}

	//std::sort(trianglesToRaster.begin(), trianglesToRaster.end(), [](TriangleV& a, TriangleV& b)
	//	{
	//		float z1 = (a.vertices[0].p.z + a.vertices[1].p.z + a.vertices[2].p.z) / 3.0f;
	//		float z2 = (b.vertices[0].p.z + b.vertices[1].p.z + b.vertices[2].p.z) / 3.0f;
	//		return z1 > z2;
	//	}
	//);

	for (TriangleV& triangle : trianglesToRaster)
	{
		TriangleV clipped[2];
		std::list<TriangleV> listTriangles;

		// Add initial triangle
		listTriangles.push_back(triangle);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				// Take triangle from front of queue
				TriangleV temp = listTriangles.front();
				TriangleV test = listTriangles.front();
				test.vertices[0].m_WorldPos = test.vertices[0].m_P;
				test.vertices[1].m_WorldPos = test.vertices[1].m_P;
				test.vertices[2].m_WorldPos = test.vertices[2].m_P;
				listTriangles.pop_front();
				nNewTriangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
				case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)window.s_Size.y - 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)window.s_Size.x - 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
				{
					clipped[w].vertices[0].m_P = temp.vertices[0].m_P;
					clipped[w].vertices[1].m_P = temp.vertices[1].m_P;
					clipped[w].vertices[2].m_P = temp.vertices[2].m_P;
					clipped[w].vertices[0].m_WorldPos = temp.vertices[0].m_WorldPos;
					clipped[w].vertices[1].m_WorldPos = temp.vertices[1].m_WorldPos;
					clipped[w].vertices[2].m_WorldPos = temp.vertices[2].m_WorldPos;
					listTriangles.push_back(clipped[w]);
				}
			}
			nNewTriangles = listTriangles.size();
		}

		for (auto& t : listTriangles)
		{
			Renderer::FillTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
			//Renderer::DrawTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
		}
	}
}