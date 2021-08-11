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

		void DrawTriangle(const GameObject& go, const Triangle& triangle);
		void ClipAgainstTheScreen();
	private:

		std::vector<TriangleV> s_TrianglesToClip;
	public:

		std::vector<Mesh*> s_Meshes;
		std::vector<GameObject*> s_GameObjects;
		Camera s_Camera;
		Time s_Time;
		glm::vec3 s_LightDir = Normalize({ 1.0f, 1.0f, 1.0f });
		Shader* s_BindedShader = nullptr;
	public:

		inline static Scene& GetInstance();
		void DrawGameObjects();

		friend class Renderer;
	};

}