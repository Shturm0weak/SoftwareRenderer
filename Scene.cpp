#include "pch.h"
#include "Scene.h"
#include "Window.h"
#include "Renderer.h"

sr::GameObject::GameObject()
{
	Scene::GetInstance().m_GameObjects.push_back(this);
}

inline sr::Scene& sr::Scene::GetInstance()
{
    static Scene scene;
    return scene;
}

void sr::Scene::DrawGameObjects()
{
	std::vector<TriangleV> trianglesToRaster;

	Window& window = Window::GetInstance();
	Scene& scene = Scene::GetInstance();
	for (GameObject* go : m_GameObjects)
	{
		if (go->m_Mesh == nullptr) continue;
		for (Triangle& triangle : go->m_Mesh->m_Triangles)
		{
			glm::vec3 points[3];
			points[0] = go->m_Mesh->m_Vertices[triangle.m_Indices[0]];
			points[1] = go->m_Mesh->m_Vertices[triangle.m_Indices[1]];
			points[2] = go->m_Mesh->m_Vertices[triangle.m_Indices[2]];

			glm::mat4 transform = go->m_Transform.m_Model * go->m_Transform.m_View * go->m_Transform.m_Scale;

			bool cullface = true;
			glm::vec3 normal = Normalize(go->m_Mesh->m_Normals[triangle.m_Indices[0]]); //can be any of three normals, because they are all the same
			glm::vec3 transformedNormal = go->m_Transform.m_View * glm::vec4(normal, 1.0f);
			transformedNormal = Normalize(transformedNormal);
			glm::vec3 transformPoint = transform * glm::vec4(points[0], 1.0f);
			cullface = glm::dot(transformedNormal, transformPoint - scene.m_Camera.m_Position) < 0.0f;

			if (cullface)
			{
				glm::vec3 p[3];
				for (size_t i = 0; i < 3; i++)
				{
					glm::vec4 pTemp = scene.m_BindedShader->m_VertexShader(
						go->m_Transform,
						transform,
						points[i],
						go->m_Mesh->m_Normals[triangle.m_Indices[i]]);

					p[i] = pTemp;
					float w = pTemp.w;
					if (w != 0.0f)
					{
						p[i].x /= w; p[i].y /= w; // p[i].z = w;
					}
				}

				p[0].x += 1.0f;
				p[1].x += 1.0f;
				p[2].x += 1.0f;

				p[0].y += 1.0f;
				p[1].y += 1.0f;
				p[2].y += 1.0f;

				p[0].x *= 0.5f * window.m_Size.x;
				p[1].x *= 0.5f * window.m_Size.x;
				p[2].x *= 0.5f * window.m_Size.x;

				p[0].y *= 0.5f * window.m_Size.y;
				p[1].y *= 0.5f * window.m_Size.y;
				p[2].y *= 0.5f * window.m_Size.y;

				trianglesToRaster.push_back(TriangleV());
				trianglesToRaster.back().vertices[0].m_P = p[0];
				trianglesToRaster.back().vertices[0].m_C = triangle.m_C[0];
				trianglesToRaster.back().vertices[1].m_P = p[1];
				trianglesToRaster.back().vertices[1].m_C = triangle.m_C[1];
				trianglesToRaster.back().vertices[2].m_P = p[2];
				trianglesToRaster.back().vertices[2].m_C = triangle.m_C[2];
				scene.m_OutParams.m_Normal = transformedNormal;
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