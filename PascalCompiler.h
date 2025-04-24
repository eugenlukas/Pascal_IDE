#pragma once

#include <string>
#include <filesystem>
#include "imgui_canvas.h"
#include <windows.h>

struct CompilationError {
	std::string filename;
	int line;
	int column;
	std::string type;
	std::string message;

	CompilationError(std::string filename, int line, int column, std::string& type, std::string& message) : filename(filename), line(line), column(column), type(type), message(message) {};
};

class PascalCompiler
{
public:
	PascalCompiler();
	PascalCompiler(const PascalCompiler&) = delete;
	PascalCompiler& operator = (const PascalCompiler&) = delete;
	~PascalCompiler();

	bool compilePascalToExe(const std::string& pascalFile, bool autoRun);
	void runCompiledExe(const std::string& pascalFile);

	bool IsErrorLine(const std::string& line);
	CompilationError GetErrorInLine(const std::string& line);

	void GenerateFormFiles(const std::vector<ImGuiCanvasElement>& elements, const std::string& formName);
	void GenerateFpcConfig();

	bool IsCompiledProgramRunning();
	void StopCompiledProgram();

	std::vector<std::string> GetErrors();
private:
	void ReplacePlaceholder(std::string& source, const std::string& placeholder, const std::string& value);
private:
	std::string currentPath = std::filesystem::current_path().string();
	std::string fpcPath = currentPath + "\\FPC\\3.2.2\\bin\\i386-win32\\fpc.exe";
	std::vector<std::string> m_errors;

	static PROCESS_INFORMATION g_compileExeProcess;
};