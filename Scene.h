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

		std::vector<Mesh*> s_Meshes;
		std::vector<GameObject*> s_GameObjects;
		Camera s_Camera;
		Time s_Time;
		glm::vec3 s_LightDir = Normalize({ 0.5f, 0.5f, 0.5f });
		Shader* s_BindedShader = nullptr;
	public:

		inline static Scene& GetInstance();
		void DrawGameObjects();
	};

}