#include "pch.h"
#include "Structs.h"
#include "Input.h"
#include "Window.h"
#include "Scene.h"

void sr::Camera::RecalculateProjectionMatrix()
{
    Window& window = Window::GetInstance();
    m_Ratio = (float)window.s_Size.x / (float)window.s_Size.y;
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), m_Roll, glm::vec3(0, 0, 1))
        * glm::rotate(glm::mat4(1.0f), m_Yaw, glm::vec3(0, 1, 0))
        * glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1, 0, 0));
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
        * rot;

    float cosPitch = cos(m_Pitch);
    m_Back.z = cos(m_Yaw) * cosPitch;
    m_Back.x = sin(m_Yaw) * cosPitch;
    m_Back.y = sin(m_Pitch);

    m_Back = Normalize(m_Back);
    m_Up = Normalize(glm::vec3(m_View[0][1], m_View[1][1], m_View[2][1]));
    m_Forward = Normalize(glm::vec3(-m_Back.x, m_Back.y, -m_Back.z));
    m_Right = Normalize(glm::cross(m_Forward, m_Up));

    m_View = glm::inverse(transform);
    m_ProjectionMartix = glm::perspective(m_Fov, m_Ratio, m_Znear, m_Zfar) * m_View;
    m_InverseProjectionMartix = glm::inverse(m_ProjectionMartix);
}

void sr::Camera::Move()
{
    Clamp();
    float speed = 5;
    float delta = Scene::GetInstance().s_Time.m_DeltaTime;
    if (Input::IsKeyDown(0x57))
    {
        m_Position += glm::vec3(-m_Back.x, m_Back.y, -m_Back.z) * delta * speed;
    }
    if (Input::IsKeyDown(0x53))
    {
        m_Position += glm::vec3(m_Back.x, -m_Back.y, m_Back.z) * delta * speed;
    }
    if (Input::IsKeyDown(0x41))
    {
        m_Position -= m_Right * delta * speed;
    }
    if (Input::IsKeyDown(0x44))
    {
        m_Position += m_Right * delta * speed;
    }
    if (Input::IsKeyDown(0x20))
    {
        m_Position += m_Up * delta * speed;
    }
    if (Input::IsKeyDown(0x43))
    {
        m_Position -= m_Up * delta * speed;
    }
    if (Input::IsKeyDown(0x25))
    {
        m_Yaw += speed * delta;
    }
    if (Input::IsKeyDown(0x27))
    {
        m_Yaw -= speed * delta;
    }
    if (Input::IsKeyDown(0x26))
    {
        m_Pitch += speed * delta;
    }
    if (Input::IsKeyDown(0x28))
    {
        m_Pitch -= speed * delta;
    }
    RecalculateProjectionMatrix();
}

void sr::Camera::Clamp()
{
    if (m_Yaw > glm::two_pi<float>() || m_Yaw < -glm::two_pi<float>())
        m_Yaw = 0;
    if (m_Pitch > glm::two_pi<float>() || m_Pitch < -glm::two_pi<float>())
        m_Pitch = 0;
}
