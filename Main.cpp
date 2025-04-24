#include "PascalCompiler.h"
#include <iostream>
#include "Window.h"
#include <windows.h>
#include "stb_image.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	//Window creation
	Window window;
	if (argc > 1)
	{
		//Path to file to directly open
		std::wstring filenameW = argv[1];
		std::string filename = std::string(filenameW.begin(), filenameW.end());
		window.CreateMainWindow(filename);
	}
	else
	{
		window.CreateMainWindow();
	}

	return 0;
}