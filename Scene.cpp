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
	for (GameObject* go : s_GameObjects)
	{
		if (go->m_Mesh == nullptr) continue;
		for (Triangle& triangle : go->m_Mesh->m_Triangles)
		{
			std::vector<TriangleV> trianglesToRaster;

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
				glm::vec3 p[3];
				for (size_t i = 0; i < 3; i++)
				{
					glm::vec4 fragPos = scene.s_BindedShader->m_VertexShader(
						go->m_Transform,
						transform,
						transformedPoints[i],
						points[i],
						transformedNormals[i]);

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

				trianglesToRaster.push_back(TriangleV());
				trianglesToRaster.back().vertices[0].m_P = p[0];
				trianglesToRaster.back().vertices[0].m_C = triangle.m_C[0];
				trianglesToRaster.back().vertices[0].m_WorldPos = transformedPoints[0];
				trianglesToRaster.back().vertices[0].m_Normal = transformedNormals[0];

				trianglesToRaster.back().vertices[1].m_P = p[1];
				trianglesToRaster.back().vertices[1].m_C = triangle.m_C[1];
				trianglesToRaster.back().vertices[1].m_WorldPos = transformedPoints[1];
				trianglesToRaster.back().vertices[1].m_Normal = transformedNormals[1];

				trianglesToRaster.back().vertices[2].m_P = p[2];
				trianglesToRaster.back().vertices[2].m_C = triangle.m_C[2];
				trianglesToRaster.back().vertices[2].m_WorldPos = transformedPoints[2];
				trianglesToRaster.back().vertices[2].m_Normal = transformedNormals[2];

				Renderer::FillTriangle(trianglesToRaster.back().vertices[0], trianglesToRaster.back().vertices[1], trianglesToRaster.back().vertices[2]);
				//Renderer::DrawTriangle(trianglesToRaster.back().vertices[0], trianglesToRaster.back().vertices[1], trianglesToRaster.back().vertices[2]);
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

	//for (TriangleV& triangle : trianglesToRaster)
	//{
	//	scene.m_OutParams.m_Normal = triangle.normal;
	//	Renderer::FillTriangle(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
	//	//Renderer::DrawTriangle(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
	//}
}