#include "pch.h"
#include "Structs.h"
#include "Input.h"
#include "Window.h"
#include "Scene.h"
#include "stb_image.h"

sr::GameObject::GameObject()
{
    Scene::GetInstance().m_GameObjects.push_back(this);
}

void sr::Camera::RecalculateProjectionMatrix()
{
    Window& window = Window::GetInstance();
    m_Ratio = (float)window.m_BitMapSize.x / (float)window.m_BitMapSize.y;
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
    float speed = 7.0f;
    float rotationSpeed = 1.0f;
    float delta = Scene::GetInstance().m_Time.m_DeltaTime;
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
        m_Yaw += rotationSpeed * delta;
    }
    if (Input::IsKeyDown(0x27))
    {
        m_Yaw -= rotationSpeed * delta;
    }
    if (Input::IsKeyDown(0x26))
    {
        m_Pitch += rotationSpeed * delta;
    }
    if (Input::IsKeyDown(0x28))
    {
        m_Pitch -= rotationSpeed * delta;
    }
    RecalculateProjectionMatrix();
}

void sr::Camera::Clamp()
{
    if (m_Yaw > glm::two_pi<float>() || m_Yaw < -glm::two_pi<float>())
    {
        m_Yaw = 0;
    }
    if (m_Pitch > glm::two_pi<float>() || m_Pitch < -glm::two_pi<float>())
    {
        m_Pitch = 0;
    }
}

sr::Texture* sr::Texture::Load(const std::string& filePath)
{
    Texture* texture = new Texture;
    texture->m_FilePath = filePath;
    stbi_set_flip_vertically_on_load(true);
    texture->m_LocalBuffer = stbi_load(filePath.c_str(), &texture->m_Size.x, &texture->m_Size.y, &texture->m_BPP, 0);
    if (texture->m_LocalBuffer == nullptr)
    {
        delete texture;
        std::cout << "Loading of <" << filePath << "> texture has failed\n";
        return nullptr;
    }
    Scene::GetInstance().m_Textures.push_back(texture);
    return texture;
}

glm::vec4 sr::Texture::Sample(glm::vec2 uv)
{
    uv = glm::clamp(uv, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    glm::ivec2 pos = uv * glm::vec2(m_Size - 1);
    uint8_t* texel;
    int position = (pos.x + pos.y * m_Size.x) * m_BPP;
    texel = &((uint8_t*)m_LocalBuffer)[position];
    return glm::vec4(texel[0], texel[1], texel[2], texel[3]) * (1.0f / 255.0f);
}

void sr::Vertex::operator*=(float invZ)
{
    m_FragPos *= invZ;
    m_Color *= invZ;
    m_UV *= invZ;
}