#pragma once

#include <unordered_map>
#include <Windows.h>

namespace sr {

    class Input {
    public:

        static bool IsMouseDoubleClicked(int keycode);
        static bool IsKeyDown(int keycode);
        static bool IsKeyPressed(int keycode);
        static bool IsKeyReleased(int keycode);
    private:

        static void ResetInput();
        static void KeyCallBack(int key, int action);
    private:

        static std::unordered_map<int, KeyProps> s_Keys; //first - keycode, second - action

        friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        friend class Window;
    };

}