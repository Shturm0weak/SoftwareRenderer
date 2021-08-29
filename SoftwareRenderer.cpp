#include "pch.h"
#include "Input.h"
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"
#include "ThreadPool.h"
#include "ObjLoader.h"

std::unordered_map<int, sr::KeyProps> sr::Input::s_Keys;
sr::ThreadPool* sr::ThreadPool::s_Instance;
bool sr::ThreadPool::m_IsInitialized;

int main(void)
{
	sr::ThreadPool::Init();
	sr::Scene& scene = sr::Scene::GetInstance();
	sr::Window& window = sr::Window::GetInstance();
	std::wstring title = L"Software renderer";
	window.Init(title, glm::ivec2(1024, 960), glm::vec2(1.0f));
	scene.m_Camera.m_Position = glm::vec3(0.0f, 0.0f, 10.0f);
	sr::Mesh* cube = objl::Loader::Load("assets/Cube.obj");
	sr::Texture* dirt = sr::Texture::Load("assets/Dirt.png");
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 5; j++)
		{
			sr::GameObject* go = sr::GameObject::Create();
			go->m_Mesh = cube;
			go->m_Transform.m_Model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2, 0.0f, j * 2));
			go->m_Transform.m_Scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
			go->m_Texture = dirt;
		}
	}

	sr::Shader shader;
	shader.m_VertexShader = [](
		const sr::Transform& transform,
		const glm::mat4& transformMat,
		const glm::vec4& transformedPosition,
		const glm::vec3& position,
		const glm::vec3& normal)
	{
		return sr::Scene::GetInstance().m_Camera.m_ProjectionMartix * transformedPosition;
	};

	shader.m_FragmentShader = [](
		const glm::vec3& worldPos,
		const glm::ivec2& fragPos,
		const glm::vec3& normal,
		const glm::ivec3& color,
		const glm::vec2& uv,
		sr::Texture* texture)
	{
		sr::Scene& scene = sr::Scene::GetInstance();
		sr::Window& window = sr::Window::GetInstance();
		
		glm::vec3 diffuseTextureColor = { 255.0f, 255.0f, 255.0f };
		//if (texture != nullptr)
		//{
			diffuseTextureColor = texture->Sample(uv);
		//}

		float colorFraction = 0.004f; //1.0f / 255.0f;
		diffuseTextureColor *= colorFraction;
		glm::vec3 ambient = 0.2f * (glm::vec3(color) * colorFraction) * diffuseTextureColor;
		scene.m_LightDir = Normalize(scene.m_LightDir);
		glm::vec3 diffuseStrength = Max(glm::dot(normal, scene.m_LightDir), 0.0f) * diffuseTextureColor;

		glm::vec3 viewDir = Normalize(scene.m_Camera.m_Position - worldPos);
		glm::vec3 reflectDir = Reflect(-scene.m_LightDir, normal);
		glm::vec3 specular = 0.4f * (float)glm::pow(Max(glm::dot(viewDir, reflectDir), 0.0f), 32) * diffuseTextureColor;

		const float gamma = 2.2f;
		ambient += (diffuseStrength + specular);
		return glm::ivec3(glm::pow(Clamp(ambient), glm::vec3(1.0 / gamma)) * 255.0f);
	};
	scene.m_BindedShader = &shader;

	float angle = 0.0f;
	double timeToReDrawFps = 0;

	while (window.m_IsRunning)
	{
		if (window.ProcessMessages() == false)
			window.m_IsRunning = false;

		double time = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		scene.m_Time.m_DeltaTime = fabs(scene.m_Time.m_LastTime - time);
		scene.m_Time.m_LastTime = time;
		scene.m_Time.m_GlobalTime += scene.m_Time.m_DeltaTime;

		if (sr::Input::IsKeyPressed(0x46))
		{
			switch (window.m_DrawBuffer)
			{
				case sr::BUFFER_STATE::SHADER:
				{
					window.m_DrawBuffer = sr::BUFFER_STATE::AMBIENT;
					break;
				}
				case sr::BUFFER_STATE::AMBIENT:
				{
					window.m_DrawBuffer = sr::BUFFER_STATE::DEPTH;
					break;
				}
				case sr::BUFFER_STATE::DEPTH:
				{
					window.m_DrawBuffer = sr::BUFFER_STATE::WIREFRAME;
					break;
				}
				case sr::BUFFER_STATE::WIREFRAME:
				{
					window.m_DrawBuffer = sr::BUFFER_STATE::SHADER;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		if (sr::Input::IsKeyDown(0x45))
		{
			window.m_ScreenPixelsInBitMapPixels += scene.m_Time.m_DeltaTime;
			window.m_ScreenPixelsInBitMapPixels = Clamp(window.m_ScreenPixelsInBitMapPixels, glm::vec2(1.0f), glm::vec2(50.0f));
			window.Resize(window.GetSize());
			std::cout << window.m_ScreenPixelsInBitMapPixels.x << std::endl;
		}
		else if (sr::Input::IsKeyDown(0x51))
		{
			window.m_ScreenPixelsInBitMapPixels -= scene.m_Time.m_DeltaTime;
			window.m_ScreenPixelsInBitMapPixels = Clamp(window.m_ScreenPixelsInBitMapPixels, glm::vec2(1.0f), glm::vec2(50.0f));
			window.Resize(window.GetSize());
			std::cout << window.m_ScreenPixelsInBitMapPixels.x << std::endl;
		}

		//if (sr::Input::IsKeyDown(0x4C))
		//{
		//	angle += scene.m_Time.m_DeltaTime;
		//	go.m_Transform.m_View = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		//}
		//else if (sr::Input::IsKeyDown(0x4B))
		//{
		//	angle -= scene.m_Time.m_DeltaTime;
		//	go.m_Transform.m_View = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		//}

		timeToReDrawFps += scene.m_Time.m_DeltaTime;
		if (timeToReDrawFps > 0.0)
		{
			window.SetTitle(title + L" " + std::to_wstring(1.0f / scene.m_Time.m_DeltaTime));
			timeToReDrawFps = 0;
		}
		
		//go.m_Transform.m_View = glm::rotate(glm::mat4(1.0f), scene.s_Time.m_GlobalTime * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		
		scene.m_Camera.Move();
		window.Clear(glm::ivec3(255));
		scene.DrawGameObjects();
		window.Update();
	}

	scene.ShutDown();

    return 0;
}