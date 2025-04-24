#pragma once

#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <fstream>
#include "TextEditor.h"
#include "PascalCompiler.h"
#include "AutoCompletionPrediction.h"
#include "FormDesignerWindow.h"
#include "CodeHelperInterface.h"
#include "FileTree.h"
#include "FileLogger.h"

struct ItemAction {
	const char* name;
	void (*action)(); // Pointer to a function for the item's action
};


class Window {
public:
	void CreateMainWindow(std::string directFileToOpen = "");
	void Update();
	void Render();
	void RenderMainMenuBar();
	void AutoPredict()
	{
		m_autoPredictionWords.clear();
		std::vector<std::string> cInterfaceResults = m_cInterface.GetResults();
		m_autoPredictionWords = cInterfaceResults /*m_autoCompletion.PredictWords(m_activeEditor, cInterfaceResults)*/;
	}

	void ButtonSave();										//Saves text(code) from current active text editor
	void ButtonOpen();										//Creates new project with main editor text from given document
	void ButtonCreateNew();									//Creates new project
	void ButtonNewSourceFile();								//Creates new text editor
	void ButtonOpenNewSourceFile(std::string path = "");	//Creates new text editor with text from given document

	void ActivateWordPredictionWindow() { m_AutocompletionWindowActive = true; m_mainEditor.SetAutocompletionWindowBoolActive(true); for (int i = 0; i < m_additionalEditors.size(); ++i) m_additionalEditors[i].SetAutocompletionWindowBoolActive(true); }
	void DeactivateWordPredictionWindow() { m_AutocompletionWindowActive = false; m_mainEditor.SetAutocompletionWindowBoolActive(false); for (int i = 0; i < m_additionalEditors.size(); ++i) m_additionalEditors[i].SetAutocompletionWindowBoolActive(true); }

	void LoadTempalte(std::string templateName);
	void OpenFile(TextEditor* editor);
	bool SaveFile();

	GLFWwindow* GetNativeWindow() const { return m_window; }

	bool GetAutopredictionWindowOpen() { return m_AutocompletionWindowActive; }

	DirectoryNode GetFileTreeRootNode() { return m_fileTreeRootNode; }

	std::vector<std::string>* GetAutocompletionWord() { return &m_autoPredictionWords; }
private:
	void LoadTextures();
	void UiToolbar();
	void CopyToClipboard(const std::string& text);
private:
	GLFWwindow* m_window;
	PascalCompiler* m_pascalCompiler = new PascalCompiler();
	TextEditor m_mainEditor;
	std::vector<TextEditor> m_additionalEditors;
	TextEditor* m_activeEditor;
	DirectoryNode m_fileTreeRootNode;
	FileTree* m_fileTree = new FileTree();
	FormDesignerWindow m_formDesigner;
	TextEditor::Coordinates cpos;
	AutoCompletionPrediction m_autoCompletion;					//Old autocompletion, now only used for appliying the prdicted word
	CodeHelperInterface m_cInterface;							//New autocompletion(Interface for Codehelpers wich is an interface to use CodeTools)

	const char* autocompletionIgnoreKeys = "\n .;,\t:"; //		"\\n" This would be two characters: '\' and 'n'

	//Textures
	unsigned int m_textureIDPlayButton;
	unsigned int m_textureIDStopButton;
	unsigned int m_textureIDToolbarBg;
	unsigned int m_textureIDClipboard;

	bool m_showMain = true;
	bool m_showConsole = true;
	bool m_showAutocompletion = true;

	bool m_AutocompletionWindowActive = false;

	std::vector<std::string> m_autoPredictionWords;

	int lastSelectedElementIndex = -1;
	int m_autocompletionWindowSelectedElementIndex = 0;

	int m_selectedCompileOptionitem = 0;


	char m_optionalCustomFileToRunFromPath[256] = "";
	char m_uiSearch[256] = "";
};