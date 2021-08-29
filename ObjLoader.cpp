#include "pch.h"
#include "ObjLoader.h"
#include "Scene.h"

sr::Mesh* objl::Loader::Load(const std::string& filePath)
{
	objl::Loader loader;
	bool loadout = loader.LoadFile(filePath);
	if (loadout)
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
				mesh->m_Triangles.push_back(sr::Triangle());
				mesh->m_Triangles.back().m_Indices[0] = curMesh.Indices[j + 0];
				mesh->m_Triangles.back().m_Indices[1] = curMesh.Indices[j + 1];
				mesh->m_Triangles.back().m_Indices[2] = curMesh.Indices[j + 2];
				mesh->m_Triangles.back().m_C[0] = glm::ivec3(225, 225, 225);
				mesh->m_Triangles.back().m_C[1] = glm::ivec3(225, 225, 225);
				mesh->m_Triangles.back().m_C[2] = glm::ivec3(225, 225, 225);
			}
			sr::Scene::GetInstance().m_Meshes.push_back(mesh);
			return mesh;
		}
	}
}