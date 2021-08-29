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

		void Init(std::wstring title = L"Software renderer", glm::ivec2 size = { 800, 600 }, glm::vec2 screenPixelsInBitMapPixels = { 1.0f, 1.0f });
		inline static Window& GetInstance();

		glm::ivec2 GetSize(HWND hWnd = GetInstance().m_HWnd);
		void SetTitle(const std::wstring& title);
		void Update();
		void Resize(glm::ivec2 size);
		void Clear(glm::ivec3 color);
		bool ProcessMessages();
	public:

		std::wstring m_Title;
		glm::vec2 m_ScreenPixelsInBitMapPixels;
		glm::ivec2 m_BitMapSize;
		BUFFER_STATE m_DrawBuffer = BUFFER_STATE::SHADER;
		bool m_IsRunning = true;
	private:

		Window() = default;
	private:

		HINSTANCE m_HInstance;
		HWND m_HWnd;
		BITMAPINFO m_BitMapInfo;
		SyncParams m_SyncParams;
		float* m_DepthBuffer = nullptr;
		void* m_BitMapMemory = nullptr;

		friend class Renderer;
		friend class Scene;
	};

}