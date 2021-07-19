#pragma once

namespace sr
{

	class Scene
	{
	private:

		Scene() = default;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		~Scene() = default;
	public:

		std::vector<Mesh*> m_Meshes;
		std::vector<GameObject*> m_GameObjects;
		glm::vec3 m_LightDir = { 0.0f, 0.0f, 1.0f };
		Camera m_Camera;
		Time m_Time;
		Shader* m_BindedShader = nullptr;
		OutParams m_OutParams;
	public:

		inline static Scene& GetInstance();
		void DrawGameObjects();
	};

}