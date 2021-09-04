#pragma once

namespace sr
{
	class Window;

	class Scene
	{
	private:

		Scene() = default;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		~Scene() = default;

		void DrawTriangle(GameObject& go, const TriangleInfo& triangle);
		void ClipAgainstTheScreen();
	private:

		std::vector<Triangle> m_TrianglesToClip;
	public:

		std::vector<Texture*> m_Textures;
		std::vector<Mesh*> m_Meshes;
		std::vector<GameObject*> m_GameObjects;
		Camera m_Camera;
		Time m_Time;
		glm::vec3 m_LightDir = Normalize({ 1.0f, 1.0f, 1.0f });
		Shader* m_BindedShader = nullptr;
	public:

		static Scene& GetInstance();
		void DrawGameObjects();
		void ShutDown();

		friend class Renderer;
	};

}