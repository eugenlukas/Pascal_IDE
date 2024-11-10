#include <iostream>
#include <windows.h>
#include "PascalCompiler.h"

PascalCompiler::PascalCompiler()
{
}

PascalCompiler::~PascalCompiler()
{
}

bool PascalCompiler::compilePascalToExe(const std::string& pascalFile, bool autoRun)
{
    // Ensure the Pascal file path is also an absolute path
    std::string pascalFilePath = std::filesystem::absolute(pascalFile).string();

    // Construct the command with PowerShell-compatible format
    std::string command = "powershell -Command \"& '" + fpcPath + "' '" + pascalFilePath + "'\"";
    OutputDebugString(command.c_str());

    // Execute the command
    int result = system(command.c_str());

    // Check the result of the compilation
    if (result != 0)
    {
        std::cerr << "Compilation failed. Please check the Pascal code for errors or verify paths." << std::endl;
        return false;
    }

    std::cout << "Compilation successful." << std::endl;

    if (autoRun)
    {
        size_t dotPos = pascalFilePath.rfind('.');
        pascalFilePath.replace(dotPos, std::string::npos, ".exe");
        std::string openExeCommand = pascalFilePath;
        system(openExeCommand.c_str()); //Open compiled program
    }

    return true;
}

void PascalCompiler::runCompiledExe(const std::string& pascalFile)
{
    std::string pascalFilePath = std::filesystem::absolute(pascalFile).string();

    size_t dotPos = pascalFilePath.rfind('.');
    pascalFilePath.replace(dotPos, std::string::npos, ".exe");
    std::string openExeCommand = pascalFilePath;
    system(openExeCommand.c_str()); //Open compiled program
}
