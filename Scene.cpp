#include "pch.h"
#include "Scene.h"
#include "Window.h"
#include "Renderer.h"

sr::Scene& sr::Scene::GetInstance()
{
    static Scene scene;
    return scene;
}

void sr::Scene::ShutDown()
{
	for (size_t i = 0; i < m_GameObjects.size(); i++)
	{
		delete m_GameObjects[i];
	}
	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		delete m_Meshes[i];
	}
	for (size_t i = 0; i < m_Textures.size(); i++)
	{
		delete m_Textures[i];
	}
}

void sr::Scene::DrawTriangle(GameObject& go, const TriangleInfo& triangle)
{
	Window& window = Window::GetInstance();

	if (go.m_Material.m_Texture == nullptr)
	{
		go.m_Material.m_Texture = m_Textures[0];
	}

	glm::vec3 points[3];
	points[0] = go.m_Mesh->m_Vertices[triangle.m_Indices[0]];
	points[1] = go.m_Mesh->m_Vertices[triangle.m_Indices[1]];
	points[2] = go.m_Mesh->m_Vertices[triangle.m_Indices[2]];

	glm::vec3 normals[3];
	normals[0] = go.m_Mesh->m_Normals[triangle.m_Indices[0]];
	normals[1] = go.m_Mesh->m_Normals[triangle.m_Indices[1]];
	normals[2] = go.m_Mesh->m_Normals[triangle.m_Indices[2]];

	glm::vec2 uvs[3];
	uvs[0] = go.m_Mesh->m_UV[triangle.m_Indices[0]];
	uvs[1] = go.m_Mesh->m_UV[triangle.m_Indices[1]];
	uvs[2] = go.m_Mesh->m_UV[triangle.m_Indices[2]];

	glm::mat4 transform = go.m_Transform.m_Model * go.m_Transform.m_View * go.m_Transform.m_Scale;

	glm::vec4 transformedPoints[3];
	transformedPoints[0] = transform * glm::vec4(points[0], 1.0f);
	transformedPoints[1] = transform * glm::vec4(points[1], 1.0f);
	transformedPoints[2] = transform * glm::vec4(points[2], 1.0f);

	glm::vec3 transformedNormals[3];
	transformedNormals[0] = Normalize(go.m_Transform.m_View * glm::vec4(normals[0], 1.0f));
	transformedNormals[1] = Normalize(go.m_Transform.m_View * glm::vec4(normals[1], 1.0f));
	transformedNormals[2] = Normalize(go.m_Transform.m_View * glm::vec4(normals[2], 1.0f));

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

	bool cullface = glm::dot(averageNormal, averageTransformedPoint - m_Camera.m_Position) < 0.0f;
	//if (cullface)
	{
		Triangle triangleToClip;
		for (size_t i = 0; i < 3; i++)
		{
			triangleToClip.vertices[i].m_WorldPos = transformedPoints[i];
			triangleToClip.vertices[i].m_Color = triangle.m_Color[i];
			triangleToClip.vertices[i].m_Normal = transformedNormals[i];
			triangleToClip.vertices[i].m_UV = uvs[i];
			triangleToClip.vertices[i].m_Material = &(go.m_Material);
		}

		Triangle clipped[2];
		for (size_t i = 0; i < 2; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				clipped[i].vertices[j].m_Material = triangleToClip.vertices[i].m_Material;
			}
		}
		
		int clippedTriangles = 0;
		clippedTriangles = TriangleClipAgainstPlane(
			(m_Camera.m_Forward * 1.0f) + m_Camera.m_Position,
			m_Camera.m_Forward,
			triangleToClip,
			clipped[0],
			clipped[1]
		);

		for (int n = 0; n < clippedTriangles; n++)
		{
			for (size_t i = 0; i < 3; i++)
			{
				glm::vec4 fragPos = m_BindedShader->m_VertexShader(
					go.m_Transform,
					transform,
					glm::vec4(clipped[n].vertices[i].m_WorldPos, 1.0f),
					points[i],
					clipped[n].vertices[i].m_Normal
				);

				float w = 1.0f / fragPos.w;
				clipped[n].vertices[i].m_FragPos = fragPos;
				clipped[n].vertices[i] *= w;
				clipped[n].vertices[i].m_FragPos.x += 1.0f;
				clipped[n].vertices[i].m_FragPos.y += 1.0f;

				clipped[n].vertices[i].m_FragPos.x *= 0.5f * window.m_BitMapSize.x;
				clipped[n].vertices[i].m_FragPos.y *= 0.5f * window.m_BitMapSize.y;

				clipped[n].vertices[i].m_FragPos.z = w;
			}
			{
				std::lock_guard lock(window.m_SyncParams.s_Mtx);
				m_TrianglesToClip.push_back(clipped[n]);
			}
		}
	}
}

void sr::Scene::ClipAgainstTheScreen()
{
	auto ClipAgainstScreenJob = [=](size_t start, size_t end)
	{
		std::list<Triangle> trianglesToRaster;
		Window& window = Window::GetInstance();
		for (size_t i = start; i < end; i++)
		{
			Triangle& triangle = m_TrianglesToClip[i];
			Triangle clipped[2];
			for (size_t i = 0; i < 2; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					clipped[i].vertices[j].m_Material = triangle.vertices[i].m_Material;
				}
			}
			trianglesToRaster.clear();
			trianglesToRaster.push_back(triangle);
			int newTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int trianglesToAdd = 0;
				while (newTriangles > 0)
				{
					glm::vec3 tempPos[3];
					Triangle test = trianglesToRaster.front();
					std::swap(test.vertices[0].m_WorldPos, test.vertices[0].m_FragPos);
					std::swap(test.vertices[1].m_WorldPos, test.vertices[1].m_FragPos);
					std::swap(test.vertices[2].m_WorldPos, test.vertices[2].m_FragPos);
					trianglesToRaster.pop_front();
					newTriangles--;

					switch (p)
					{
					case 0:	trianglesToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 1:	trianglesToAdd = TriangleClipAgainstPlane({ 0.0f, (float)window.m_BitMapSize.y, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 2:	trianglesToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 3:	trianglesToAdd = TriangleClipAgainstPlane({ (float)window.m_BitMapSize.x, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					}

					for (int w = 0; w < trianglesToAdd; w++)
					{
						std::swap(clipped[w].vertices[0].m_FragPos, clipped[w].vertices[0].m_WorldPos);
						std::swap(clipped[w].vertices[1].m_FragPos, clipped[w].vertices[1].m_WorldPos);
						std::swap(clipped[w].vertices[2].m_FragPos, clipped[w].vertices[2].m_WorldPos);
						trianglesToRaster.push_back(clipped[w]);
					}
				}
				newTriangles = trianglesToRaster.size();
			}

			for (auto& t : trianglesToRaster)
			{
				if (window.m_DrawBuffer == BUFFERSTATE::WIREFRAME)
				{
					Renderer::DrawTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
				}
				else
				{
					Renderer::FillTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
				}
			}
		}
	};

	Window& window = Window::GetInstance();
	size_t numThreads = ThreadPool::GetInstance().GetAmountOfThreads();
	size_t size = m_TrianglesToClip.size();
	float dif = (float)size / (float)numThreads;
	
	for (size_t k = 0; k < numThreads - 1; k++)
	{
		window.m_SyncParams.s_Ready[k] = false;
		ThreadPool::GetInstance().Enqueue([=] {
			ClipAgainstScreenJob(k * dif, k * dif + dif);
			Window::GetInstance().m_SyncParams.ThreadFinished(k);
		});
	}

	window.m_SyncParams.s_Ready[numThreads - 1] = false;
	ThreadPool::GetInstance().Enqueue([=] {
		ClipAgainstScreenJob(((numThreads - 1) * dif),size);
		Window::GetInstance().m_SyncParams.ThreadFinished(numThreads - 1);
	});

	window.m_SyncParams.WaitForAllThreads();
}

void sr::Scene::DrawGameObjects()
{
	auto DrawTrianglesJob = [=](GameObject& go, size_t start, size_t end)
	{
		for (size_t i = start; i < end; i++)
		{
			TriangleInfo& triangle = go.m_Mesh->m_Triangles[i];
			DrawTriangle(go, triangle);
		}
	};

	Window& window = Window::GetInstance();
	m_TrianglesToClip.clear();
	size_t numThreads = ThreadPool::GetInstance().GetAmountOfThreads();
	for (size_t j = 0; j < m_GameObjects.size(); j++)
	{
		if (m_GameObjects[j]->m_Mesh == nullptr) continue;
		size_t size = m_GameObjects[j]->m_Mesh->m_Triangles.size();
		float dif = (float)size / (float)numThreads;

		for (size_t k = 0; k < numThreads - 1; k++)
		{
			window.m_SyncParams.s_Ready[k] = false;
			ThreadPool::GetInstance().Enqueue([=] {
				DrawTrianglesJob(*m_GameObjects[j], k * dif, k * dif + dif);
				Window::GetInstance().m_SyncParams.ThreadFinished(k);
			});
		}

		window.m_SyncParams.s_Ready[numThreads - 1] = false;
		ThreadPool::GetInstance().Enqueue([=] {
			DrawTrianglesJob(*m_GameObjects[j], (numThreads - 1) * dif, size);
			Window::GetInstance().m_SyncParams.ThreadFinished(numThreads - 1);
		});

		window.m_SyncParams.WaitForAllThreads();
		ClipAgainstTheScreen();
	}
}