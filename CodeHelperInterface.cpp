#include "CodeHelperInterface.h"
#include <iostream>
#include <Windows.h>
#include "FileLogger.h"
#include "StringHelper.h"

CodeHelperInterface::CodeHelperInterface()
{
    if (!std::filesystem::exists("CodeHelpers\\codetools.config"))
        GenerateConfig();
}

CodeHelperInterface::~CodeHelperInterface(){}

void CodeHelperInterface::UpdateLastInput(TextEditor* currentEditor)
{
    if (SaveTmpFile(currentEditor))
        PredictWords(currentEditor);
}

void CodeHelperInterface::PredictWords(TextEditor* editor)
{
    if (m_tmpFilePathUri.empty() || m_tmpFilePathUri == " ")
    {
        OutputDebugString("uri empty");
        LOG("Code Helps uri empty");
        return;
    }

    // Create pipes for stdin and stdout
    HANDLE hReadOut, hWriteOut;
    HANDLE hReadIn, hWriteIn;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE }; // Inheritable handles

    // Create pipe for stdout
    CreatePipe(&hReadOut, &hWriteOut, &saAttr, 0);
    SetHandleInformation(hReadOut, HANDLE_FLAG_INHERIT, 0); // Make read end non-inheritable

    // Create pipe for stdin
    CreatePipe(&hReadIn, &hWriteIn, &saAttr, 0);
    SetHandleInformation(hWriteIn, HANDLE_FLAG_INHERIT, 0); // Make write end non-inheritable

    // Setup STARTUPINFO to redirect handles and hide window
    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWriteOut;
    si.hStdError = hWriteOut;
    si.hStdInput = hReadIn;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    std::string command = "CodeHelpers\\AutocompletionHelper.exe";

    if (!CreateProcessA(
        nullptr,
        command.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        "CodeHelpers",
        &si,
        &pi
    ))
    {
        OutputDebugString("Failed to start code helpers!");
        LOG("Error: Failed to start code helpers!");
        return;
    }

    WaitForInputIdle(pi.hProcess, 200); // Wait for the process to be ready

    // Close unused pipe ends
    CloseHandle(hReadIn);
    CloseHandle(hWriteOut);


    // Write to Pascal stdin
    std::string line = std::to_string(editor->GetCursorPosition().mLine + 1);            //mLine + 1 to get to the correct line (TextEditor gives one less than actual?!)
    std::string column = std::to_string((editor->GetConvertedColumn() + 1) - 1);  //mLine + 1 to get to the correct line (TextEditor gives one less than actual?!), -1 to get one char behind, would cancel out, but who cares
    std::string input = "completion " + m_tmpFilePathUri + " " + line + " " + column + "\n";
    //OutputDebugString(input.c_str());
    DWORD written;
    WriteFile(hWriteIn, input.c_str(), input.size(), &written, nullptr);
    CloseHandle(hWriteIn); // Signal EOF

    // Read output from Pascal stdout
    char buffer[1024];
    DWORD read;
    std::string output;
    while (ReadFile(hReadOut, buffer, sizeof(buffer), &read, nullptr) && read > 0)
    {
        output.append(buffer, read);
    }

    //OutputDebugString(output.c_str() + '\n');

    LOG("Code Helper Output:\n---\n" + output + "\n---");
    results.clear();
    if (!output.empty())
    {
        // DON'T TOUCH THIS CONVRESION OR EVERYTHING BREAKS
        std::string jsonString = SanitizeStringLiteral(output);
        size_t jsonStart = jsonString.find("{");
        std::string jsonFinal = jsonString.substr(jsonStart);

        // Parse the JSON string into a nlohmann::json object
        nlohmann::json json_obj = nlohmann::json::parse(jsonFinal);

        // Access and add elements to result list from the parsed JSON
        for (const auto& item : json_obj["completions"])
        {
            for (const auto& inner : item.items())
            {
                results.push_back(inner.value()["result"]);
            }
        }
    }

    // Cleanup
    CloseHandle(hReadOut);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

std::string CodeHelperInterface::toUri(const std::string& filePath)
{
    std::string uri = "file:///" + filePath;
    std::replace(uri.begin(), uri.end(), '\\', '/');  // Convert backslashes to slashes
    return uri;
}

bool CodeHelperInterface::SaveTmpFile(TextEditor* editor)
{
    m_tmpFileSaved = true;

    std::filesystem::create_directories(m_tmpFilePath.parent_path());  // create the Eleazer folder if needed

    // Open the file in output mode, truncating it to overwrite existing content
    std::fstream file;
    file.open(m_tmpFilePath, std::ios::out | std::ios::trunc);  // open file to write

    if (file.is_open())
    {
        std::string editorText = editor->GetText();  // Get all text from the editor
        file << editorText;  // Write the text to the file

        file.close();  // Close the file when done
        return true;
    }
    else
    {
        // Handle file opening failure
        LOG("Code Helpers failed to open the file for saving.");
        return false;
    }
}

void CodeHelperInterface::GenerateConfig()
{
    std::ofstream cfgFile("CodeHelpers\\codetools.config");
    if (!cfgFile.is_open())
    {
        LOG("Error: Could not create code helpers config file.");
        return;
    }

    std::string file = R""""(<?xml version="1.0" encoding="UTF-8"?>
<CONFIG>
  <CodeToolsOptions>
    <FPC>
      <CompilerPath Value="{applicationPath}\FPC\3.2.2\bin\i386-win32\fpc.exe"/>
      <SrcDir Value="{applicationPath}\FPC\3.2.2\source"/>
    </FPC>
    <Lazarus>
      <SrcDir Value="F:\EleazerTestFiles\AutocompletionHelper\~\pascal\lazarus"/>
      <LCLWidgetType Value="gtk2"/>
    </Lazarus>
    <FPCConfigCaches Count="0">
    </FPCConfigCaches>
    <FPCSrcDirCaches Count="0">
    </FPCSrcDirCaches>
  </CodeToolsOptions>
</CONFIG>
)"""";

          ReplacePlaceholder(file, "{applicationPath}", std::filesystem::current_path().string());

          cfgFile.clear();
          cfgFile << file;
          cfgFile.close();
}

void CodeHelperInterface::ReplacePlaceholder(std::string& source, const std::string& placeholder, const std::string& value)
{
    size_t pos = 0;
    while ((pos = source.find(placeholder, pos)) != std::string::npos)
    {
        source.replace(pos, placeholder.length(), value);
        pos += value.length();
    }
}
