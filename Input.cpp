#include "pch.h"
#include "Input.h"

bool sr::Input::IsMouseDoubleClicked(int keycode)
{
    auto keyIter = s_Keys.find(keycode);
    if (keyIter != s_Keys.end())
    {
        if (keyIter->second.m_Action == (int)INPUT_STATE::DOUBLECLICK)
        {
            return true;
        }
    }
    return false;
}

bool sr::Input::IsKeyDown(int keycode)
{
    //Note: it works, but I think the second approach is better because the refresh rate is much higher
    //auto keyIter = s_Keys.find(keycode);
    //if (keyIter != s_Keys.end())
    //{
    //    if (keyIter->second.m_Action == (int)INPUT_STATE::DOWN || 
    //        keyIter->second.m_Action == (int)INPUT_STATE::PRESSED)
    //    {
    //        return true;
    //    }
    //}
    //return false;

    return (GetKeyState(keycode) & 0x8000);
}

bool sr::Input::IsKeyPressed(int keycode)
{
    auto keyIter = s_Keys.find(keycode);
    if (keyIter != s_Keys.end())
    {
        if (keyIter->second.m_Action == (int)INPUT_STATE::PRESSED)
        {
            return true;
        }
    }
    return false;
}

bool sr::Input::IsKeyReleased(int keycode)
{
    auto keyIter = s_Keys.find(keycode);
    if (keyIter != s_Keys.end())
    {
        if (keyIter->second.m_Action == (int)INPUT_STATE::UP)
        {
            return true;
        }
    }
    return false;
}

void sr::Input::ResetInput()
{
    for (auto keyIter = s_Keys.begin(); keyIter != s_Keys.end(); keyIter++)
    {
        keyIter->second.m_Action = (int)INPUT_STATE::NONE;
    }
}

void sr::Input::KeyCallBack(int key, int action)
{
    auto keyIter = s_Keys.find(key);
    if (keyIter != s_Keys.end())
    {
        if (action == (int)INPUT_STATE::PRESSED &&
            keyIter->second.m_PreviousAction == (int)INPUT_STATE::PRESSED)
        {
            keyIter->second.m_Action = (int)INPUT_STATE::NONE;
        }
        else
        {
            keyIter->second = action;
            keyIter->second.m_PreviousAction = keyIter->second.m_Action;
        }
    }
    else
    {
        s_Keys.insert(std::make_pair(key, KeyProps(action)));
    }
}