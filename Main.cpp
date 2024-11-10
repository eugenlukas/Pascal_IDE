#include "PascalCompiler.h"
#include <iostream>
#include "Window.h"
#include <windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Window window;
	window.CreateMainWindow();

	return 0;
}

//void test()
//{
//	PascalCompiler* pascalCompiler = new PascalCompiler();
//
//	std::string pasFilepath;
//	std::cout << "Name of file: ";
//	std::cin >> pasFilepath;
//
//	if (pascalCompiler->compilePascalToExe((std::filesystem::current_path() / (pasFilepath + ".pas")).string()))
//	{
//	}
//	else
//	{
//	}
//
//	std::string openExeCommand = std::filesystem::current_path().string() + "\\" + pasFilepath + ".exe";
//	system(openExeCommand.c_str()); //Open compiled program
//}