#pragma once

#include <string>
#include <filesystem>

class PascalCompiler
{
public:
	PascalCompiler();
	PascalCompiler(const PascalCompiler&) = delete;
	PascalCompiler& operator = (const PascalCompiler&) = delete;
	~PascalCompiler();

	bool compilePascalToExe(const std::string& pascalFile, bool autoRun);
	void runCompiledExe(const std::string& pascalFile);

	std::string GetError();
private:
	std::string currentPath = std::filesystem::current_path().string();
	std::string fpcPath = currentPath + "\\FPC\\bin\\i386-win32\\ppcrossx64.exe";
	std::string m_error = "";
};