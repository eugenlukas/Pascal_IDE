#pragma once
#include <vector>
#include <windows.h>
#include <iostream>
#include <filesystem>

class FormDesignerWindow {
public:
	~FormDesignerWindow();

	void Render();
private:
	static HWND g_lazarusWindow;
	static HWND g_mainApplicationWindow;
	static PROCESS_INFORMATION g_lazarusProcess;

	void LaunchLazarus();
	void SynchronizeWindowState();
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

	std::string currentPath = std::filesystem::current_path().string();
	std::string lazarusPath = currentPath + "\\Lazarus\\lazarus.exe";
};