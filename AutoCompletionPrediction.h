#pragma once
#include <iostream>
#include "TextEditor.h"
#include <set>

class AutoCompletionPrediction {
public:
	std::vector<std::string> PredictWords(TextEditor* editor, std::vector<std::string> cInterfaceWords);
	void ApplyPredictedWord(const std::string& predictedWord, TextEditor* editor);
private:
	TextEditor* m_pTextEditor;
private:
	std::vector<std::string> LoadWords();
	std::string GetCurrentTypedWord();
	int LevenshteinDistance(const std::string& a, const std::string& b);

	std::unordered_map<std::string, int> CalculateWordFrequencies(const std::vector<std::string>& words);
	std::vector<std::string> GetSuggestions(const std::string& inputText,
		const std::unordered_map<std::string, int>& wordFrequencies);

	void CheckForAutoAddition(const std::string& predictedWord);
private:
	std::string m_currentTypedWord;
	std::vector<std::string> m_cInterfaceWords;

	const std::set<char> m_delimiters = { ' ', '(', ')', '[', ']', '{', '}', ';', ',', '\t', '=' };
};