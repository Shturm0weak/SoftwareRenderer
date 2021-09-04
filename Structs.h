#pragma once

namespace sr
{

	enum class BUFFERSTATE
	{
		SHADER,
		DEPTH,
		WIREFRAME
	};

	struct Texture
	{
		std::string m_FilePath;
		glm::ivec2 m_Size = { 0, 0 };
		unsigned char* m_LocalBuffer = nullptr;
		int m_BPP = 0;
		static Texture* Load(const std::string& filePath);
		glm::vec3 Sample(glm::vec2 uv);
		~Texture() { delete[] m_LocalBuffer; }
	};

	struct Material
	{
		float m_Specular = 0.1f;
		float m_Ambient = 0.2f;
		Texture* m_Texture = nullptr;
	};

	struct Camera
	{
		glm::mat4 m_ProjectionMartix;
		glm::mat4 m_InverseProjectionMartix;
		glm::mat4 m_View;
		glm::vec3 m_Back;
		glm::vec3 m_Forward;
		glm::vec3 m_Right;
		glm::vec3 m_Up;
		glm::vec3 m_Position;
		float m_Roll = 0.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;
		float m_Ratio = 0.0f;
		float m_Zoom = 1.0f;
		float m_Znear = 0.1f;
		float m_Zfar = 1000.0f;
		float m_Fov = glm::radians(60.0f);

		void RecalculateProjectionMatrix();
		void Move();
		void Clamp();
	};

	struct TriangleInfo
	{
		glm::uvec3 m_Indices;
		glm::ivec3 m_Color[3]; //color
	};

	struct Vertex
	{
		glm::vec3 m_FragPos; //position in screen space
		glm::vec3 m_WorldPos; //interpolated position in world space
		glm::vec3 m_Normal; //interpolated transformed normal
		glm::ivec3 m_Color; //interpolated color
		glm::vec2 m_UV; //interpolated uv coordinates
		Material* m_Material = nullptr;
	};

	struct Triangle
	{
		Vertex vertices[3];
	};

	struct Mesh
	{
		std::vector<TriangleInfo> m_Triangles;
		std::vector<glm::vec3> m_Vertices;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec2> m_UV;
		glm::ivec3 m_Color = { 255, 255, 255 };
	};

	struct Transform
	{
		glm::mat4 m_Model = glm::mat4(1.0f);
		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Scale = glm::mat4(1.0f);
	};

	struct Time
	{
		double m_DeltaTime = 0.0f;
		double m_LastTime = 0.0f;
		double m_GlobalTime = 0.0f;
	};

	struct GameObject
	{
		Transform m_Transform;
		Material m_Material;
		Mesh* m_Mesh = nullptr;
		GameObject();
	};

	struct Shader
	{
		std::function<glm::vec4 (const Transform&, const glm::mat4&, const glm::vec4&, const glm::vec3&, const glm::vec3&)> m_VertexShader;
		std::function<glm::vec3 (const glm::vec3&, const glm::ivec2&, const glm::vec3&, const glm::vec3&, const glm::vec2&, const Material&)> m_FragmentShader;
	};

	enum class INPUTSTATE
	{
		NONE = -1,
		PRESSED = 0,
		//DOWN = 1, Don't need this, because the down state we get with GetKeyState function
		UP = 2,
		DOUBLECLICK = 3
	};

	struct KeyProps
	{
		int m_Action;
		int m_PreviousAction;

		KeyProps(int action)
		{
			m_Action = action;
			m_PreviousAction = (int)INPUTSTATE::PRESSED;
		}
	};

}