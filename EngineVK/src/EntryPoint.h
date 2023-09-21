#pragma once

#include "Core/Application.h"

extern LearningVK::Application* LearningVK::CreateApplication();

int main(int argc, char** argv)
{
	LearningVK::Application* app = LearningVK::CreateApplication();
	app->Run();
	delete app;
}