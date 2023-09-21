#pragma once

namespace LearningVK {

	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnDestruct() = 0;
	public:
		bool Running = true;
	};

	Application* CreateApplication();
}