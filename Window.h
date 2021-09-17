#pragma once

#include <Windows.h>
#include <condition_variable>

namespace sr
{

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	class Window
	{
	public:

		static Window& GetInstance();
		glm::ivec2 GetSize(HWND hWnd = GetInstance().m_HWnd);
		void Init(
			const std::wstring& title = L"Software renderer",
			const glm::ivec2& size = { 800, 600 },
			const glm::vec2& screenPixelsInBitMapPixels = { 1.0f, 1.0f }
		);
		void SetTitle(const std::wstring& title);
		void Update();
		void Resize(const glm::ivec2& size);
		void Clear(const glm::ivec4& color);
		bool ProcessMessages();
	public:

		std::wstring m_Title;
		glm::vec2 m_ScreenPixelsInBitMapPixels;
		glm::ivec2 m_BitMapSize;
		BUFFERSTATE m_DrawBuffer = BUFFERSTATE::SHADER;
		bool m_IsRunning = true;
	private:

		Window() = default;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();
	private:

		HINSTANCE m_HInstance;
		HWND m_HWnd;
		BITMAPINFO m_BitMapInfo;
		HBITMAP m_HBitMap;
		BLENDFUNCTION m_BlendFunction;
		SyncParams m_SyncParams;
		float* m_DepthBuffer = nullptr;
		void* m_BitMapMemory = nullptr;
		bool m_DiscradPixel = false;

		friend class Renderer;
		friend class Scene;
	};

}