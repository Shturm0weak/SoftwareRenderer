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

		std::wstring s_Title;
		glm::ivec2 s_Size;
		BUFFER_STATE s_DrawBuffer = BUFFER_STATE::SHADER;
		bool s_IsRunning = true;
	private:

		Window() = default;
	private:

		HINSTANCE s_HInstance;
		HWND s_HWnd;
		BITMAPINFO s_BitMapInfo;
		glm::ivec2 s_BitMapSize;
		float* s_DepthBuffer = nullptr;
		void* s_BitMapMemory = nullptr;

		friend class Renderer;
	};

}