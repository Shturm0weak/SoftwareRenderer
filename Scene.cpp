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

void sr::Scene::DrawTriangle(const GameObject& go, const Triangle& triangle)
{
	Window& window = Window::GetInstance();

	glm::vec3 points[3];
	points[0] = go.m_Mesh->m_Vertices[triangle.m_Indices[0]];
	points[1] = go.m_Mesh->m_Vertices[triangle.m_Indices[1]];
	points[2] = go.m_Mesh->m_Vertices[triangle.m_Indices[2]];

	glm::vec3 normals[3];
	normals[0] = go.m_Mesh->m_Normals[triangle.m_Indices[0]];
	normals[1] = go.m_Mesh->m_Normals[triangle.m_Indices[1]];
	normals[2] = go.m_Mesh->m_Normals[triangle.m_Indices[2]];

	glm::mat4 transform = go.m_Transform.m_Model * go.m_Transform.m_View * go.m_Transform.m_Scale;

	glm::vec4 transformedPoints[3];
	transformedPoints[0] = transform * glm::vec4(points[0], 1.0f);
	transformedPoints[1] = transform * glm::vec4(points[1], 1.0f);
	transformedPoints[2] = transform * glm::vec4(points[2], 1.0f);

	glm::vec3 transformedNormals[3];
	transformedNormals[0] = go.m_Transform.m_View * glm::vec4(normals[0], 1.0f);
	transformedNormals[1] = go.m_Transform.m_View * glm::vec4(normals[1], 1.0f);
	transformedNormals[2] = go.m_Transform.m_View * glm::vec4(normals[2], 1.0f);

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

	bool cullface = glm::dot(averageNormal, averageTransformedPoint - s_Camera.m_Position) < 0.0f;
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
		clippedTriangles = TriangleClipAgainstPlane(
			(s_Camera.m_Forward * 1.0f) + s_Camera.m_Position,
			s_Camera.m_Forward,
			triangleToClip,
			clipped[0],
			clipped[1]);

		for (int n = 0; n < clippedTriangles; n++)
		{
			glm::vec3 p[3];
			for (size_t i = 0; i < 3; i++)
			{
				glm::vec4 fragPos = s_BindedShader->m_VertexShader(
					go.m_Transform,
					transform,
					glm::vec4(clipped[n].vertices[i].m_WorldPos, 1.0f),
					points[i],
					clipped[n].vertices[i].m_Normal);

				p[i] = fragPos;
				p[i] *= 1.0 / fragPos.w;
			}

			p[0].x += 1.0f;
			p[1].x += 1.0f;
			p[2].x += 1.0f;

			p[0].y += 1.0f;
			p[1].y += 1.0f;
			p[2].y += 1.0f;

			p[0].x *= 0.5f * window.s_BitMapSize.x;
			p[1].x *= 0.5f * window.s_BitMapSize.x;
			p[2].x *= 0.5f * window.s_BitMapSize.x;

			p[0].y *= 0.5f * window.s_BitMapSize.y;
			p[1].y *= 0.5f * window.s_BitMapSize.y;
			p[2].y *= 0.5f * window.s_BitMapSize.y;

			clipped[n].vertices[0].m_P = p[0];
			clipped[n].vertices[1].m_P = p[1];
			clipped[n].vertices[2].m_P = p[2];

			{
				std::lock_guard lock(window.s_SyncParams.s_Mtx);
				s_TrianglesToClip.push_back(clipped[n]);
			}
		}
	}
}

void sr::Scene::ClipAgainstTheScreen()
{
	Window& window = Window::GetInstance();
	size_t numThreads = ThreadPool::GetInstance().GetAmountOfThreads();
	size_t size = s_TrianglesToClip.size();
	float dif = (float)size / (float)numThreads;
	for (size_t k = 0; k < numThreads - 1; k++)
	{
		window.s_SyncParams.s_Ready[k] = false;
		ThreadPool::GetInstance().Enqueue([=] {
			Window& window = Window::GetInstance();
			std::list<TriangleV> trianglesToRaster;
			uint32_t thisSegmentOfObjectsV = k * dif + dif;
			for (size_t i = k * dif; i < thisSegmentOfObjectsV; i++)
			{
				TriangleV triangle = s_TrianglesToClip[i];
				TriangleV clipped[2];
				trianglesToRaster.clear();
				trianglesToRaster.push_back(triangle);
				int nNewTriangles = 1;

				for (int p = 0; p < 4; p++)
				{
					int nTrisToAdd = 0;
					while (nNewTriangles > 0)
					{
						glm::vec3 tempPos[3];
						TriangleV test = trianglesToRaster.front();
						tempPos[0] = test.vertices[0].m_WorldPos;
						tempPos[1] = test.vertices[1].m_WorldPos;
						tempPos[2] = test.vertices[2].m_WorldPos;
						test.vertices[0].m_WorldPos = test.vertices[0].m_P;
						test.vertices[1].m_WorldPos = test.vertices[1].m_P;
						test.vertices[2].m_WorldPos = test.vertices[2].m_P;
						test.vertices[0].m_P = tempPos[0];
						test.vertices[1].m_P = tempPos[1];
						test.vertices[2].m_P = tempPos[2];
						trianglesToRaster.pop_front();
						nNewTriangles--;
						switch (p)
						{
						case 0:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
						case 1:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, (float)window.s_BitMapSize.y - 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
						case 2:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
						case 3:	nTrisToAdd = TriangleClipAgainstPlane({ (float)window.s_BitMapSize.x - 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
						}

						for (int w = 0; w < nTrisToAdd; w++)
						{
							tempPos[0] = clipped[w].vertices[0].m_P;
							tempPos[1] = clipped[w].vertices[1].m_P;
							tempPos[2] = clipped[w].vertices[2].m_P;
							clipped[w].vertices[0].m_P = clipped[w].vertices[0].m_WorldPos;
							clipped[w].vertices[1].m_P = clipped[w].vertices[1].m_WorldPos;
							clipped[w].vertices[2].m_P = clipped[w].vertices[2].m_WorldPos;
							clipped[w].vertices[0].m_WorldPos = tempPos[0];
							clipped[w].vertices[1].m_WorldPos = tempPos[1];
							clipped[w].vertices[2].m_WorldPos = tempPos[2];
							trianglesToRaster.push_back(clipped[w]);
						}
					}
					nNewTriangles = trianglesToRaster.size();
				}

				Window& window = Window::GetInstance();
				for (auto& t : trianglesToRaster)
				{
					if (window.s_DrawBuffer == BUFFER_STATE::WIREFRAME)
					{
						Renderer::DrawTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
					}
					else
					{
						Renderer::FillTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
					}
				}
			}
			{
				window.s_SyncParams.ThreadFinished(k);
			}
		});
	}
	window.s_SyncParams.s_Ready[numThreads - 1] = false;
	ThreadPool::GetInstance().Enqueue([=] {
		Window& window = Window::GetInstance();
		std::list<TriangleV> trianglesToRaster;
		for (size_t i = (numThreads - 1) * dif; i < size; i++)
		{
			TriangleV triangle = s_TrianglesToClip[i];
			TriangleV clipped[2];
			trianglesToRaster.clear();
			trianglesToRaster.push_back(triangle);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 1;
				while (nNewTriangles > 0)
				{
					glm::vec3 tempPos[3];
					TriangleV test = trianglesToRaster.front();
					tempPos[0] = test.vertices[0].m_WorldPos;
					tempPos[1] = test.vertices[1].m_WorldPos;
					tempPos[2] = test.vertices[2].m_WorldPos;
					test.vertices[0].m_WorldPos = test.vertices[0].m_P;
					test.vertices[1].m_WorldPos = test.vertices[1].m_P;
					test.vertices[2].m_WorldPos = test.vertices[2].m_P;
					test.vertices[0].m_P = tempPos[0];
					test.vertices[1].m_P = tempPos[1];
					test.vertices[2].m_P = tempPos[2];
					trianglesToRaster.pop_front();
					nNewTriangles--;
					switch (p)
					{
					case 0:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 1:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, (float)window.s_BitMapSize.y - 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 2:	nTrisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					case 3:	nTrisToAdd = TriangleClipAgainstPlane({ (float)window.s_BitMapSize.x - 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], true); break;
					}

					for (int w = 0; w < nTrisToAdd; w++)
					{
						tempPos[0] = clipped[w].vertices[0].m_P;
						tempPos[1] = clipped[w].vertices[1].m_P;
						tempPos[2] = clipped[w].vertices[2].m_P;
						clipped[w].vertices[0].m_P = clipped[w].vertices[0].m_WorldPos;
						clipped[w].vertices[1].m_P = clipped[w].vertices[1].m_WorldPos;
						clipped[w].vertices[2].m_P = clipped[w].vertices[2].m_WorldPos;
						clipped[w].vertices[0].m_WorldPos = tempPos[0];
						clipped[w].vertices[1].m_WorldPos = tempPos[1];
						clipped[w].vertices[2].m_WorldPos = tempPos[2];
						trianglesToRaster.push_back(clipped[w]);
					}
				}
				nNewTriangles = trianglesToRaster.size();
			}

			if (Window::GetInstance().s_DrawBuffer == BUFFER_STATE::WIREFRAME)
			{
				for (auto& t : trianglesToRaster)
				{
					Renderer::DrawTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
				}
			}
			else
			{
				for (auto& t : trianglesToRaster)
				{
					Renderer::FillTriangle(t.vertices[0], t.vertices[1], t.vertices[2]);
				}
			}
		}
		{
			window.s_SyncParams.ThreadFinished(numThreads - 1);
		}
	});

	window.s_SyncParams.WaitForAllThreads();
}

void sr::Scene::DrawGameObjects()
{
	Window& window = Window::GetInstance();
	s_TrianglesToClip.clear();
	size_t numThreads = ThreadPool::GetInstance().GetAmountOfThreads();
	for (size_t j = 0; j < s_GameObjects.size(); j++)
	{
		if (s_GameObjects[j]->m_Mesh == nullptr) continue;
		size_t size = s_GameObjects[j]->m_Mesh->m_Triangles.size();
		float dif = (float)size / (float)numThreads;
		for (size_t k = 0; k < numThreads - 1; k++)
		{
			window.s_SyncParams.s_Ready[k] = false;
			ThreadPool::GetInstance().Enqueue([=] {
				uint32_t thisSegmentOfObjectsV = k * dif + dif;
				for (size_t i = k * dif; i < thisSegmentOfObjectsV; i++)
				{
					Triangle& triangle = s_GameObjects[j]->m_Mesh->m_Triangles[i];
					DrawTriangle(*s_GameObjects[j], triangle);
				}
				{
					Window::GetInstance().s_SyncParams.ThreadFinished(k);
				}
			});
		}
		window.s_SyncParams.s_Ready[numThreads - 1] = false;
		ThreadPool::GetInstance().Enqueue([=] {
			for (size_t i = (numThreads - 1) * dif; i < size; i++)
			{
				Triangle& triangle = s_GameObjects[j]->m_Mesh->m_Triangles[i];
				DrawTriangle(*s_GameObjects[j], triangle);
			}
			{
				Window::GetInstance().s_SyncParams.ThreadFinished(numThreads - 1);
			}
		});

		window.s_SyncParams.WaitForAllThreads();
	}
	ClipAgainstTheScreen();
}