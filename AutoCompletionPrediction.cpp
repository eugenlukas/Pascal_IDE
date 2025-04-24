#include "AutoCompletionPrediction.h"
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <limits>

std::vector<std::string> AutoCompletionPrediction::PredictWords(TextEditor* editor, std::vector<std::string> cInterfaceWords)
{
    m_pTextEditor = editor;
    m_cInterfaceWords = cInterfaceWords;

    auto wordFrequencies = CalculateWordFrequencies(LoadWords());
    auto suggestions = GetSuggestions(m_currentTypedWord, wordFrequencies);

    return suggestions;
}

void AutoCompletionPrediction::ApplyPredictedWord(const std::string& predictedWord, TextEditor* editor)
{
    m_pTextEditor = editor;
    m_currentTypedWord = GetCurrentTypedWord();

    // Get the current cursor position
    auto cursorPos = editor->GetCursorPosition();

    // Get the text of the current line
    std::string lineText = editor->GetCurrentLineText();
    int cursorColumn = editor->GetCursorPosition().mColumn;

    // Find start of the word
    int startColumn = cursorColumn;
    while (startColumn > 0 && m_delimiters.count(lineText[startColumn - 1]) == 0)
    {
        --startColumn;
    }

    // Now select from the start of the word to the cursor
    editor->SetSelection(
        TextEditor::Coordinates(cursorPos.mLine, startColumn),
        TextEditor::Coordinates(cursorPos.mLine, cursorColumn)
    );


    // Delete the selected text (only the word itself, not any preceding whitespace)
    editor->Delete();

    // Insert the predicted word
    editor->InsertText(predictedWord, true);

    // Move the cursor to the end of the inserted word
    TextEditor::Coordinates newCursorPosition(
        cursorPos.mLine,  // Same line
        startColumn + static_cast<int>(predictedWord.length() + 1) // New column position at the end of the word
    );
    editor->SetCursorPosition(newCursorPosition);

    CheckForAutoAddition(predictedWord);
}


std::vector<std::string> AutoCompletionPrediction::LoadWords()
{
    std::string text = m_pTextEditor->GetText();
    std::vector<std::string> words;
    std::string word;

    //Load predefined words
    for (std::string keyword : m_pTextEditor->GetLanguageDefinition().mKeywords)
        text.append(" " + keyword);

    //Load CodeHelper found words
    for (size_t i = 0; i < m_cInterfaceWords.size(); i++)
        if (!m_pTextEditor->GetLanguageDefinition().mKeywords.contains(m_cInterfaceWords[i]))
            text.append(" " + m_cInterfaceWords[i]);

    //Load rest of words
    for (char ch : text)
    {
        // Check if the character is alphanumeric or an underscore (part of a word)
        if (std::isalnum(ch) || ch == '_')
        {
            word += ch;
        }
        else
        {
            // If we hit a symbol, add the completed word to the list if it's non-empty
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
            // Ignore symbols (do not add them as separate words)
        }
    }

    // Add the last word if any remains
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

std::string AutoCompletionPrediction::GetCurrentTypedWord()
{
    std::string lineText = m_pTextEditor->GetCurrentLineText();
    int cursorColumn = m_pTextEditor->GetCursorPosition().mColumn;

    // Guard against out-of-bounds
    if (cursorColumn > static_cast<int>(lineText.length()))
        cursorColumn = static_cast<int>(lineText.length());

    std::string word;
    for (int i = cursorColumn - 1; i >= 0; --i)
    {
        char ch = lineText[i];
        if (m_delimiters.count(ch) > 0)
        {
            break;
        }

        word = ch + word;
    }

    return word;
}

// Function to calculate Levenshtein Distance between two strings
int AutoCompletionPrediction::LevenshteinDistance(const std::string& a, const std::string& b)
{
    int lenA = a.size();
    int lenB = b.size();
    std::vector<std::vector<int>> dp(lenA + 1, std::vector<int>(lenB + 1));

    for (int i = 0; i <= lenA; ++i) dp[i][0] = i;
    for (int j = 0; j <= lenB; ++j) dp[0][j] = j;

    for (int i = 1; i <= lenA; ++i)
    {
        for (int j = 1; j <= lenB; ++j)
        {
            if (a[i - 1] == b[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = 1 + std::min({ dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1] });
            }
        }
    }

    return dp[lenA][lenB];
}

// Function to calculate word frequencies
std::unordered_map<std::string, int> AutoCompletionPrediction::CalculateWordFrequencies(const std::vector<std::string>& words)
{
    std::unordered_map<std::string, int> wordFrequencies;

    for (const auto& word : words)
    {
        ++wordFrequencies[word];
    }

    return wordFrequencies;
}

// Function to get the top N suggestions based on similarity to input text
std::vector<std::string> AutoCompletionPrediction::GetSuggestions(const std::string& inputText,
    const std::unordered_map<std::string, int>& wordFrequencies)
{
    std::vector<std::tuple<std::string, int, bool>> matchingWords; // (word, distance, isPrefixMatch)

    // Calculate similarity for each word and keep the closest matches, excluding exact match of `inputText`
    for (const auto& [word, frequency] : wordFrequencies)
    {
        if (word == inputText)
        {
            continue;  // Skip if the word is exactly the same as the inputText
        }

        int distance = LevenshteinDistance(inputText, word);
        bool isPrefixMatch = word.find(inputText) == 0;  // True if word starts with inputText
        matchingWords.emplace_back(word, distance, isPrefixMatch);
    }

    // Sort with these priorities:
    // 1. Prefix matches first (true > false)
    // 2. Lower edit distance
    // 3. Higher frequency (in the case of a tie in distance)
    std::sort(matchingWords.begin(), matchingWords.end(),
        [&](const auto& a, const auto& b) {
            // Unpack the tuple for clarity
            const auto& [wordA, distanceA, isPrefixMatchA] = a;
            const auto& [wordB, distanceB, isPrefixMatchB] = b;

            if (isPrefixMatchA != isPrefixMatchB)
            {
                return isPrefixMatchA > isPrefixMatchB; // Prefix matches come first
            }
            else if (distanceA != distanceB)
            {
                return distanceA < distanceB;           // Closer edit distances come next
            }
            else
            {
                return wordFrequencies.at(wordA) > wordFrequencies.at(wordB); // Higher frequency in case of tie
            }
        });

    // Collect only the top N word suggestions
    std::vector<std::string> topSuggestions;
    for (int i = 0; i < std::min(10, static_cast<int>(matchingWords.size())); ++i)
    {
        topSuggestions.push_back(std::get<0>(matchingWords[i])); // Add only the word part
    }

    return topSuggestions;
}

void AutoCompletionPrediction::CheckForAutoAddition(const std::string& predictedWord)
{
    //Tryed a switch but dosn't work (?)
    if (predictedWord == "begin")
    {
        TextEditor::Coordinates cursorPos = m_pTextEditor->GetCursorPosition();
        m_pTextEditor->InsertText("\nend;", true);
        m_pTextEditor->SetCursorPosition(cursorPos);
    }
}
