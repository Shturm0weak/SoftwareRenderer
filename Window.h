#pragma once

#include <Windows.h>

namespace sr
{

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	class Window
	{
	public:

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		void Init(std::wstring title, glm::ivec2 size);
		inline static Window& GetInstance();

		void Resize(glm::ivec2 size);
		void Update();
		void Clear(glm::ivec3 color);
		bool ProcessMessages();
	public:

		std::wstring m_Title;
		glm::ivec2 m_Size;
		bool m_IsRunning = true;
		BUFFER_STATE m_DrawBuffer = BUFFER_STATE::SHADER;
	private:

		Window() = default;
	private:

		HINSTANCE m_HInstance;
		HWND m_HWnd;
		BITMAPINFO m_BitMapInfo;
		glm::ivec2 m_BitMapSize;
		float* m_DepthBuffer = nullptr;
		void* m_BitMapMemory = nullptr;
		friend class Renderer;
	};

}