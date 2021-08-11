#pragma once

#include <Windows.h>
#include <condition_variable>

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

		void SetTitle(const std::wstring& title);
		void Update();
		void Resize(glm::ivec2 size);
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

		SyncParams s_SyncParams;

		friend class Renderer;
		friend class Scene;
	};

}