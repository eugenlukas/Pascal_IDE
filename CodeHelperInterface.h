#pragma once
#include <fstream>
#include "TextEditor.h"
#include "json.hpp"
using json = nlohmann::json;
using timePoint = std::chrono::steady_clock::time_point;

class CodeHelperInterface
{
public:
	CodeHelperInterface();
	~CodeHelperInterface();

	void UpdateLastInput(TextEditor* currentEditor);

	void PredictWords(TextEditor* editor);

	bool GetTmpFileSaved() const { return m_tmpFileSaved; }
	std::vector<std::string> GetResults() const { return results; }
private:
	std::string toUri(const std::string& filePath);
	bool SaveTmpFile(TextEditor* editor);
	void GenerateConfig();
	void ReplacePlaceholder(std::string& source, const std::string& placeholder, const std::string& value);
private:
	timePoint timeStart;
	timePoint timeEnd;
	bool m_tmpFileSaved = true;

	std::vector<std::string> results;

	std::filesystem::path m_tmpFilePath = std::filesystem::temp_directory_path() / "Eleazer" / "currentFileText.pas";
	std::string m_tmpFilePathUri = "file:///" + m_tmpFilePath.generic_string();
};