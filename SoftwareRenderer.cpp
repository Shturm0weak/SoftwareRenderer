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
	window.Init(title, glm::ivec2(1024, 960), glm::vec2(20.0f));
	scene.m_Camera.m_Position = glm::vec3(-3.2f, 4.0f, 0.0f);
	scene.m_Camera.m_Yaw = glm::radians(-20.0f);
	scene.m_Camera.m_Pitch = glm::radians(-20.0f);
	sr::Mesh* model = objl::Loader::Load("assets/Sphere.obj");

	sr::Texture::Load("assets/WhiteTexture.png");
	sr::Texture* texture = sr::Texture::Load("assets/Earth.png");
	/*for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 5; j++)
		{
			sr::GameObject* go = sr::GameObject::Create();
			go->m_Mesh = cube;
			go->m_Transform.m_Model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2, 0.0f, j * 2));
			go->m_Transform.m_Scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
			go->m_Texture = dirt;
		}
	}*/

	sr::GameObject* go = new sr::GameObject;
	go->m_Mesh = model;
	//go->m_Transform.m_Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//go->m_Transform.m_View = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	go->m_Transform.m_Scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
	go->m_Material.m_Ambient = 0.4f;
	go->m_Material.m_Specular = 0.8f;
	go->m_Material.m_Texture = texture;

	/*sr::GameObject* go1 = new sr::GameObject;
	go1->m_Mesh = model;
	go1->m_Transform.m_Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	go1->m_Transform.m_View = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	go1->m_Transform.m_View = glm::rotate(go1->m_Transform.m_View, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	go1->m_Transform.m_Scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
	go1->m_Material.m_Ambient = 0.4f;
	go1->m_Material.m_Specular = 0.2f;
	go1->m_Material.m_Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	go1->m_Material.m_Texture = texture;*/

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
		const glm::vec4& color,
		const glm::vec2& uv,
		const sr::Material& material)
	{
		sr::Scene& scene = sr::Scene::GetInstance();
		sr::Window& window = sr::Window::GetInstance();
	
		glm::vec4 diffuseTextureColor = material.m_Texture->Sample(uv);

		glm::vec4 ambient = material.m_Ambient * material.m_Color * color * diffuseTextureColor;
		scene.m_LightDir = Normalize(scene.m_LightDir);
		float diffuseStrength = Max(glm::dot(normal, scene.m_LightDir), 0.0f);
		
		glm::vec3 viewDir = Normalize(scene.m_Camera.m_Position - worldPos);
		glm::vec3 reflectDir = Normalize(Reflect(-scene.m_LightDir, normal));
		float specular = material.m_Specular * (float)glm::pow(Max(glm::dot(viewDir, reflectDir), 0.0f), 32);
		
		const float gamma = 2.2f;
		ambient += (diffuseStrength + specular) * diffuseTextureColor;
		ambient = sr::Renderer::GammaCorrection(ambient, gamma);
		return glm::clamp(ambient, glm::vec4(0.0f), glm::vec4(1.0f));
	};
	scene.m_BindedShader = &shader;

	float angle = 0.0f;
	double timeToReDrawFps = 0.0;

	while (window.m_IsRunning)
	{
		if (window.ProcessMessages() == false)
		{
			window.m_IsRunning = false;
		}

		double time = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		scene.m_Time.m_DeltaTime = fabs(scene.m_Time.m_LastTime - time);
		scene.m_Time.m_LastTime = time;
		scene.m_Time.m_GlobalTime += scene.m_Time.m_DeltaTime;

		if (sr::Input::IsKeyPressed(0x46))
		{
			switch (window.m_DrawBuffer)
			{
				case sr::BUFFERSTATE::SHADER:
				{
					window.m_DrawBuffer = sr::BUFFERSTATE::DEPTH;
					break;
				}
				case sr::BUFFERSTATE::DEPTH:
				{
					window.m_DrawBuffer = sr::BUFFERSTATE::WIREFRAME;
					break;
				}
				case sr::BUFFERSTATE::WIREFRAME:
				{
					window.m_DrawBuffer = sr::BUFFERSTATE::SHADER;
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
			window.m_ScreenPixelsInBitMapPixels = glm::clamp(window.m_ScreenPixelsInBitMapPixels, glm::vec2(1.0f), glm::vec2(2.0f));
			window.Resize(window.GetSize());
			std::cout << window.m_ScreenPixelsInBitMapPixels.x << std::endl;
		}
		else if (sr::Input::IsKeyDown(0x51))
		{
			window.m_ScreenPixelsInBitMapPixels -= scene.m_Time.m_DeltaTime;
			window.m_ScreenPixelsInBitMapPixels = glm::clamp(window.m_ScreenPixelsInBitMapPixels, glm::vec2(1.0f), glm::vec2(2.0f));
			window.Resize(window.GetSize());
			std::cout << window.m_ScreenPixelsInBitMapPixels.x << std::endl;
		}

		if (sr::Input::IsKeyPressed(0x4C))
		{
			
		}
		else if (sr::Input::IsKeyPressed(0x4B))
		{
			
		}

		timeToReDrawFps += scene.m_Time.m_DeltaTime;
		if (timeToReDrawFps > 0.0)
		{
			window.SetTitle(title + L" " + std::to_wstring(1.0f / scene.m_Time.m_DeltaTime));
			timeToReDrawFps = 0;
		}
		
		//go->m_Transform.m_View = glm::rotate(glm::mat4(1.0f), (float)scene.m_Time.m_GlobalTime, glm::vec3(1.0f, 0.0f, 0.0f));
		
		scene.m_Camera.Move();
		window.Clear(glm::ivec4(255));
		scene.DrawGameObjects();
		window.Update();
	}

	scene.ShutDown();

    return 0;
}