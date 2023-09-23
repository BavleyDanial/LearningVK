#include <vkpch.h>

#include "Application.h"

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