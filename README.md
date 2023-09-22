# LearningVK
This is a small project that I started to learn Vulkan :)
It will end up as a game engine, though it will be a small incomplete one, for example, I don't think I will add any audio support in the near future.
Keep in mind I am a year 12 student, so work on this project will be limited for now. This is not a game engine to use but rather one that you can just check out and enjoy reading :)

# Roadmap
- [ ] Draw first triangle using Vulkan
- [ ] Abstract Window
- [ ] Abstract Vulkan
- [ ] Add Logging support
- [ ] Add Debugging utilities
- [ ] Add Input system

# Building
This engine uses Premake as its build configuration system, so in theory, this will allow it to work with any toolchain/compiler. However, I've only tested it with Visual Studio 17 and MSVC.
This engine is also only supported on Windows at the moment but I do plan to work on Linux compatibility.

# Building on Windows
Run the ```GenerateProjects.bat``` file to generate a visual studio solution or if you have premake installed locally (and in your PATH environment) then you can run ```premake5.exe [action] --cc=desired_compiler``` from the root directory.

