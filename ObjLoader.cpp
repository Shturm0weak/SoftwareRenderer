#include "pch.h"
#include "ObjLoader.h"
#include "Scene.h"

sr::Mesh* objl::Loader::Load(const std::string& filePath)
{
	objl::Loader loader;
	bool loadOut = loader.LoadFile(filePath);
	if (loadOut)
	{
		for (size_t i = 0; i < loader.LoadedMeshes.size(); i++)
		{
			objl::Mesh curMesh = loader.LoadedMeshes[i];
			sr::Mesh* mesh = new sr::Mesh;
			for (size_t j = 0; j < curMesh.Vertices.size(); j++)
			{
				mesh->m_Vertices.push_back({ curMesh.Vertices[j].Position.X, curMesh.Vertices[j].Position.Y, curMesh.Vertices[j].Position.Z });
				mesh->m_Normals.push_back({ curMesh.Vertices[j].Normal.X, curMesh.Vertices[j].Normal.Y, curMesh.Vertices[j].Normal.Z });
				mesh->m_UV.push_back({ curMesh.Vertices[j].TextureCoordinate.X, curMesh.Vertices[j].TextureCoordinate.Y });
			}

			for (size_t j = 0; j < curMesh.Indices.size(); j += 3)
			{
				sr::TriangleInfo triangle;
				triangle.m_Indices[0] = curMesh.Indices[j + 0];
				triangle.m_Indices[1] = curMesh.Indices[j + 1];
				triangle.m_Indices[2] = curMesh.Indices[j + 2];
				triangle.m_Color[0] = glm::vec4(1.0f);
				triangle.m_Color[1] = glm::vec4(1.0f);
				triangle.m_Color[2] = glm::vec4(1.0f);
				mesh->m_Triangles.push_back(triangle);
			}
			sr::Scene::GetInstance().m_Meshes.push_back(mesh);
			return mesh;
		}
	}
}