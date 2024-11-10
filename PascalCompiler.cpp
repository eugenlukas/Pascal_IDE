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
    //Reset old error
    m_error = "";

    // Ensure the Pascal file path is also an absolute path
    std::string pascalFilePath = std::filesystem::absolute(pascalFile).string();

    // Construct the command with PowerShell-compatible format
    std::string command = "powershell -Command \"& '" + fpcPath + "' '" + pascalFilePath + "'\"";
    OutputDebugString((command + "\n").c_str());

    // Run the command and capture the result
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Error running PowerShell command." << std::endl;
        return false;
    }

    // Read the output and errors from the pipe
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        m_error += buffer;  // Append output/error to m_error
    }

    // Close the pipe
    int result = _pclose(pipe);

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

std::string PascalCompiler::GetError()
{
    return m_error;
}
