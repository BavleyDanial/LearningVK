#include <vkpch.h>

#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

namespace LearningVK
{

	Application::Application()
	{
		
	}

	Application::~Application()
	{

	}

	void Application::Run()
    {
		OnInit();

		while (Running)
			OnUpdate();

		OnDestruct();
	}

}