#pragma once
#include <string>
#include "Window.h"

class FileDialogs {
public:
	static std::string OpenFile(const char* filter, const Window& window);
	static std::string SaveFile(const char* filter, const Window& window);
};