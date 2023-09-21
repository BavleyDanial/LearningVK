#include "EngineVK.h"
#include "EntryPoint.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>

struct WindowProperties
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    std::string Title = "";
};

class SandboxVK : public LearningVK::Application
{
public:
    GLFWwindow* window = nullptr;
    WindowProperties windowProps;
public:
	void OnInit() override
	{
        InitWindow();
	}

	void OnUpdate() override
	{
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }

        Running = false;
	}

    void OnDestruct() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
private:
    void InitWindow()
    {
        glfwInit();

        windowProps.Width = 800;
        windowProps.Height = 600;
        windowProps.Title = "Hello Vulkan!";
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(windowProps.Width, windowProps.Height, windowProps.Title.c_str(), nullptr, nullptr);
    }
};

LearningVK::Application* LearningVK::CreateApplication()
{
	return new SandboxVK();
}