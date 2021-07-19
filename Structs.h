#pragma once

namespace sr
{

	enum class BUFFER_STATE
	{
		SHADER = 0,
		AMBIENT = 1,
		DEPTH = 2,
	};

	struct Camera
	{
		float m_Roll = 0.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;
		float m_Ratio = 0.0f;
		float m_Zoom = 1.0f;
		float m_Znear = 0.1f;
		float m_Zfar = 1000.0f;
		float m_Fov = glm::radians(60.0f);
		glm::mat4 m_ProjectionMartix;
		glm::mat4 m_View;
		glm::vec3 m_Back;
		glm::vec3 m_Forward;
		glm::vec3 m_Right;
		glm::vec3 m_Up;
		glm::vec3 m_Position;

		void RecalculateProjectionMatrix();
		void Move();
		void Clamp();
	};

	struct Triangle
	{
		glm::uvec3 m_Indices;
		glm::ivec3 m_C[3]; //color
	};

	struct Vertex
	{
		glm::vec3 m_P; //position
		glm::ivec3 m_C; //color
	};

	struct Point
	{
		glm::ivec2 m_P; //position
		glm::ivec3 m_C; //color
		float m_Z; //interpolated z component for a depth buffer

		Point(glm::ivec2 p = glm::ivec2(0), glm::ivec3 c = glm::ivec3(255, 255, 255), float z = 0.0f)
		{
			this->m_P = p;
			this->m_C = c;
			this->m_Z = z;
		}

		Point(const Vertex& vertex)
		{
			m_P = vertex.m_P;
			m_C = vertex.m_C;
		}
	};

	struct TriangleV
	{
		Vertex vertices[3];
	};

	struct Mesh
	{
		std::vector<Triangle> m_Triangles;
		std::vector<glm::vec3> m_Vertices;
		std::vector<glm::vec3> m_Normals;
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
		float m_DeltaTime = 0.0f;
		float m_LastTime = 0.0f;
		float m_GlobalTime = 0.0f;
	};

	struct GameObject
	{
		Transform m_Transform;
		Mesh* m_Mesh = nullptr;

		GameObject();
	};

	struct Shader
	{
		std::function<glm::vec4 (const Transform&, const glm::mat4&, const glm::vec3&, const glm::vec3&)> m_VertexShader;
		std::function<glm::ivec3 (const glm::ivec2&, const glm::vec3&, const glm::ivec3&)> m_FragmentShader;
	};

	struct OutParams
	{
		glm::vec3 m_Normal;
	};

	enum class INPUT_STATE
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
			m_PreviousAction = (int)INPUT_STATE::PRESSED;
		}
	};

}