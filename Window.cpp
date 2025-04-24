#define STB_IMAGE_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Window.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "FileDialogs.h"
#include <Windows.h>
#include "imguitablabel.h"
#include "stb_image.h"
#include "imgui_canvas.h"
#include "TextureLoader.h"
#include "ThemeManager.h"
#include "StringHelper.h"

void Window::CreateMainWindow(std::string directFileToOpen)
{
	LOG("Started");
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	m_window = glfwCreateWindow(1920, 1080, "Eleazar", NULL, NULL);

	// Error check if the window fails to create
	if (m_window == NULL)
	{
		LOG("Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	// Change window icon
	int iconWidth, iconHeight, iconChannels;
	std::string iconPath = (std::filesystem::current_path() / "Icon.png").string();
	unsigned char* iconPixels = stbi_load(iconPath.c_str(), &iconWidth, &iconHeight, &iconChannels, 4);
	GLFWimage images[1];
	images[0].width = iconWidth;
	images[0].height = iconHeight;
	images[0].pixels = iconPixels;
	glfwSetWindowIcon(m_window, 1, images);

	// Maximize the window
	glfwMaximizeWindow(m_window);

	// Introduce the window into the current context
	glfwMakeContextCurrent(m_window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 800, 800);

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ThemeManager::SetApplicationDarkStyle();

	// Load Textures
	LoadTextures();
	LOG("Loaded Textures");

	// Initialize Main Text Editor
	auto lang = TextEditor::LanguageDefinition::Pascal();
	m_activeEditor = &m_mainEditor;
	m_mainEditor.addOnCharTypedCallback([this](ImWchar aChar) { m_cInterface.UpdateLastInput(m_activeEditor); });
	m_mainEditor.addOnCharTypedCallback([this](ImWchar aChar) { if (!strchr(autocompletionIgnoreKeys, aChar) && !m_activeEditor->IsBetweenQuotes()) this->AutoPredict(); });
	m_mainEditor.addOnCharTypedCallback([this](ImWchar aChar) { if (!strchr(autocompletionIgnoreKeys, aChar) && !m_activeEditor->IsBetweenQuotes()) this->ActivateWordPredictionWindow(); });
	m_mainEditor.SetLanguageDefinition(lang);

	// Initialize File Tree
	m_fileTree->addOnCharTypedCallback([this](std::string path) {this->ButtonOpenNewSourceFile(path); });

	//Load old saved ui
	ImGui::LoadIniSettingsFromDisk("imgui.ini");

	//Open given file directly if there is one or create default
	if (!(directFileToOpen == ""))
	{
		m_mainEditor.SetFilepath(directFileToOpen);
		std::filesystem::path fsPath(directFileToOpen);
		std::string parentPath = fsPath.parent_path().string();
		m_fileTreeRootNode = m_fileTree->CreateDirectryNodeTreeFromPath(parentPath);
		OpenFile(&m_mainEditor);
	}
	else
		LoadTempalte("new_file");

	// Main while loop
	while (!glfwWindowShouldClose(m_window))
	{
		Update();
		Render();
	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// Delete window before ending the program
	glfwDestroyWindow(m_window);
	// Terminate GLFW before ending the program
	glfwTerminate();
}

void Window::LoadTextures()
{
	TextureLoader::LoadTexture("PlayButton.png", &m_textureIDPlayButton);
	TextureLoader::LoadTexture("StopButton.png", &m_textureIDStopButton);
	TextureLoader::LoadTexture("ToolbarBg.png", &m_textureIDToolbarBg);
	TextureLoader::LoadTexture("Clipboard.png", &m_textureIDClipboard);
}

void Window::Update()
{
	if (m_activeEditor)
		cpos = m_activeEditor->GetCursorPosition();

	//Hotkey/Shortcut Listener
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
		ButtonSave();
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_O))
		ButtonOpen();
	if (ImGui::IsKeyPressed(ImGuiKey_F9, false))
	{
		if (m_selectedCompileOptionitem == 0)
			m_pascalCompiler->compilePascalToExe(m_activeEditor->GetFilepath(), true);
		else
			m_pascalCompiler->compilePascalToExe(m_optionalCustomFileToRunFromPath, true);
	}
}

void Window::Render()
{
	// Specify the color of the background
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Begin new ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	RenderMainMenuBar();

	// Docking configuration
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	 
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		window_flags |= ImGuiWindowFlags_NoBackground; // Transparent background
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", nullptr, window_flags);

	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;

	ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	// Docking setup (ensuring persistent layout)
	if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(dockspace_id); // Reset dock
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);
		ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

		ImGuiID dock_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.75f, nullptr, &dockspace_id);
		ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dock_top);

		ImGui::DockBuilderDockWindow("Code Editor", dock_top);
		ImGui::DockBuilderDockWindow("Console", dock_bottom);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGui::End();

	// Code Editor Window
	if (m_showMain)
	{
		ImGui::Begin("Code Editor");
		if (ImGui::BeginTabBar("EditorTabs"))
		{
			if (ImGui::BeginTabItem(m_mainEditor.GetFilepath().empty() ? "Main file" : m_mainEditor.GetFilepath().c_str()))
			{
				m_activeEditor = &m_mainEditor;

				ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s",
					cpos.mLine + 1, m_mainEditor.GetConvertedColumn() + 1, m_mainEditor.GetTotalLines(),
					m_mainEditor.IsOverwrite() ? "Ovr" : "Ins",
					m_mainEditor.CanUndo() ? "*" : " ",
					m_mainEditor.GetLanguageDefinition().mName.c_str(),
					m_mainEditor.GetFilepath().c_str());

				m_mainEditor.Render("Editor");
				ImGui::EndTabItem();
			}

			for (size_t i = 0; i < m_additionalEditors.size();)
			{
				bool open = true;
				std::string editorName = "Editor " + std::to_string(i + 1);

				if (ImGui::BeginTabItem(m_additionalEditors[i].GetFilepath().empty() ? editorName.c_str() : m_additionalEditors[i].GetFilepath().c_str(), &open))
				{
					m_activeEditor = &m_additionalEditors[i];

					ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s",
						cpos.mLine + 1, cpos.mColumn + 1, m_additionalEditors[i].GetTotalLines(),
						m_additionalEditors[i].IsOverwrite() ? "Ovr" : "Ins",
						m_additionalEditors[i].CanUndo() ? "*" : " ",
						m_additionalEditors[i].GetLanguageDefinition().mName.c_str(),
						m_additionalEditors[i].GetFilepath().c_str());

					m_additionalEditors[i].Render(editorName.c_str());
					ImGui::EndTabItem();

					if (!open)
					{
						m_additionalEditors.erase(m_additionalEditors.begin() + i);
						continue;
					}
				}
				++i;
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	// Console Window
	if (m_showConsole)
	{
		ImGui::Begin("Console");
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 170));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		for (std::string error : m_pascalCompiler->GetErrors())
		{
			std::string convertedError = SanitizeStringLiteral(error);
			std::string buttonID = "##";
			buttonID.append(error.c_str());
			if (ImGui::ImageButton(buttonID.c_str(), m_textureIDClipboard, ImVec2(15, 15)))
				CopyToClipboard(error);

			ImGui::SameLine();

			if (m_pascalCompiler->IsErrorLine(convertedError))
				ImGui::TextColored(ImVec4(255, 0, 0, 255), error.c_str());
			else
				ImGui::Text(error.c_str());
		}

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		ImGui::End();
	}

	// Autocompletion Window
	if (m_showAutocompletion && m_AutocompletionWindowActive)
	{
		static int highlightedIndex = 0;

		ImVec2 pos = m_activeEditor->GetScreenPositionFromTextPosition(m_activeEditor->GetCursorPosition());
		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::Begin("Auto word prediction", &m_AutocompletionWindowActive, ImGuiWindowFlags_NoFocusOnAppearing);

		HWND hwnd = (HWND)ImGui::GetCurrentContext();
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			highlightedIndex = (highlightedIndex + 1) % m_autoPredictionWords.size();
		else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			highlightedIndex = (highlightedIndex - 1 + m_autoPredictionWords.size()) % m_autoPredictionWords.size();

		for (size_t i = 0; i < m_autoPredictionWords.size(); ++i)
		{
			bool isSelected = (i == highlightedIndex);
			if (ImGui::Selectable(m_autoPredictionWords[i].c_str(), isSelected))
			{
				m_autoCompletion.ApplyPredictedWord(m_autoPredictionWords[i], m_activeEditor);
				DeactivateWordPredictionWindow();
				break;
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			m_autoCompletion.ApplyPredictedWord(m_autoPredictionWords[highlightedIndex], m_activeEditor);
			DeactivateWordPredictionWindow();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (ImGui::IsKeyPressed(ImGuiKey_Comma) && ImGui::IsKeyDown(ImGuiKey_LeftShift))
			|| ImGui::IsKeyPressed(ImGuiKey_Semicolon) || ImGui::IsKeyPressed(ImGuiKey_LeftBracket))	//cant use ImGui::IsKeyPressed(ImGuiKey_Semicolon) only, because in qwertz they must press shift+comma
			DeactivateWordPredictionWindow();

		ImGui::End();
	}

	UiToolbar();

	//File Tree
	ImGui::Begin("Folder structure", nullptr);
	if (m_fileTreeRootNode.FullPath != "")
	{
		m_fileTree->Render(m_fileTreeRootNode, true);
	}
	else
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float size = ImGui::CalcTextSize("Open File").x + style.FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * 0.5f;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
		if (ImGui::Button("Open File"))
			ButtonOpen();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
		if (ImGui::Button("New File"))
			ButtonCreateNew();
	}
	ImGui::End();

	//Render formdesigner window
	//m_FormDesigner.Render();

	// Renders the ImGUI elements
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap the back buffer with the front buffer
	glfwSwapBuffers(m_window);
	// Take care of all GLFW events
	glfwPollEvents();
}

void Window::UiToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.5f));

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	float size = ImGui::GetWindowHeight() - 4.0f;
	float buttonSize = size - 10;
	float buttonSpacing = 25.0f; // Space between the two buttons
	float backgroundWidth = (buttonSize * 2) + buttonSpacing; // Make background match buttons
	float backgroundHeight = buttonSize + 6.0f; // Add slight padding for aesthetics

	// ---- Center Background Independently ----
	float bgPosX = (ImGui::GetContentRegionMax().x * 0.5f) - (backgroundWidth * 0.5f);
	float bgPosY = (ImGui::GetContentRegionAvail().y * 0.5f) - (backgroundHeight * 0.5f);
	ImGui::SetCursorPos(ImVec2(bgPosX, bgPosY + 2.5f));
	ImGui::Image(m_textureIDToolbarBg, ImVec2(backgroundWidth, backgroundHeight));

	// ---- Center Buttons Independently ----
	float buttonPosX = (ImGui::GetContentRegionMax().x * 0.5f) - ((buttonSize * 2 + buttonSpacing) * 0.5f);
	float buttonPosY = bgPosY + ((backgroundHeight - buttonSize) * 0.5f); // Align vertically in background

	ImGui::SetCursorPos(ImVec2(buttonPosX, buttonPosY));

	// Check if compiled program is running
	bool CompiledProgramIsRunning = m_pascalCompiler->IsCompiledProgramRunning();

	// Grey out start button when the compiled program is already running or not file is opend
	if (CompiledProgramIsRunning || m_mainEditor.GetFilepath().empty())
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	//Render PlayButton
	if (ImGui::ImageButton("PlayButtonTex", m_textureIDPlayButton, ImVec2(buttonSize, buttonSize)))
	{
		if (m_selectedCompileOptionitem == 0)
			m_pascalCompiler->compilePascalToExe(m_mainEditor.GetFilepath(), true);
		else
			m_pascalCompiler->compilePascalToExe(m_optionalCustomFileToRunFromPath, true);
	}

	// Grey out start button when the compiled program is already running or not file is opend
	if (CompiledProgramIsRunning || m_mainEditor.GetFilepath().empty())
	{
		// Pop greyed out styling
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::SameLine();

	// Grey out stop button when the compiled program is not running
	if (!CompiledProgramIsRunning)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	//Render StopButton
	if (ImGui::ImageButton("StopButtonTex", m_textureIDStopButton, ImVec2(buttonSize, buttonSize)))
	{
		m_pascalCompiler->StopCompiledProgram();
	}

	// Grey out stop button when the compiled program is not running
	if (!CompiledProgramIsRunning)
	{
		// Pop greyed out styling
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::Separator();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::End();
}

void Window::CopyToClipboard(const std::string& text)
{
	// Open the clipboard
	if (!OpenClipboard(nullptr)) return;

	// Empty the clipboard
	EmptyClipboard();

	// Allocate global memory for the text (plus null terminator)
	HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
	if (!hGlob)
	{
		CloseClipboard();
		return;
	}

	// Copy the string into the global memory
	memcpy(GlobalLock(hGlob), text.c_str(), text.size() + 1);
	GlobalUnlock(hGlob);

	// Set clipboard data
	SetClipboardData(CF_TEXT, hGlob);

	// Close the clipboard
	CloseClipboard();
}

void Window::RenderMainMenuBar()
{
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0, 0, 0, 0));

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Create new project"))
			{
				if (ImGui::MenuItem("Program"))
				{
					//m_Editor.SetProgramType(TextEditor::ProgramType::Program);
					ButtonCreateNew();
				}
				if (ImGui::MenuItem("Application"))
				{
					//m_Editor.SetProgramType(TextEditor::ProgramType::Application);
					ButtonCreateNew();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Open project", "Ctrl+O"))
			{
				ButtonOpen();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (m_mainEditor.GetFilepath() == "")
				ImGui::BeginDisabled();
			if (ImGui::MenuItem("Create new source file"))
			{
				ButtonNewSourceFile();
			}
			if (ImGui::MenuItem("Open new source file"))
			{
				ButtonOpenNewSourceFile();
			}
			if (m_mainEditor.GetFilepath() == "")
				ImGui::EndDisabled();
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				ButtonSave();
			}
			if (ImGui::MenuItem("Save as.."))
			{
				std::string savePath = FileDialogs::SaveFile("", *this);
				if (savePath != "")
				{
					m_activeEditor->SetFilepath(savePath);
					SaveFile();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run/Compile"))
		{
			//Compile file option
			static const char* items[]{ "Current File","Custom File" };
			
			ImGui::Combo("Options", &m_selectedCompileOptionitem, items, IM_ARRAYSIZE(items));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("Select if you want to start your compile from the current file you have opened or a custom file");
			}
			//window to show custom compile file
			if (m_selectedCompileOptionitem == 1)
			{
				ImGui::InputTextWithHint("", "Custom File Path", m_optionalCustomFileToRunFromPath, 256);
				ImGui::SameLine();
				if (ImGui::Button("Select.."))
				{
					strncpy_s(m_optionalCustomFileToRunFromPath, FileDialogs::OpenFile("", *this).c_str(), sizeof(m_optionalCustomFileToRunFromPath) - 1);
					// Ensure null termination
					m_optionalCustomFileToRunFromPath[sizeof(m_optionalCustomFileToRunFromPath) - 1] = '\0';
				}
			}

			//Compile/Run buttons
			if (ImGui::MenuItem("Compile and Run", "F9"))
			{
				if (m_selectedCompileOptionitem == 0)
					m_pascalCompiler->compilePascalToExe(m_mainEditor.GetFilepath(), true);
				else
					m_pascalCompiler->compilePascalToExe(m_optionalCustomFileToRunFromPath, true);
			}
			if (ImGui::MenuItem("Run"))
			{
				if (m_selectedCompileOptionitem == 0)
					m_pascalCompiler->runCompiledExe(m_mainEditor.GetFilepath());
				else
					m_pascalCompiler->runCompiledExe(m_optionalCustomFileToRunFromPath);
			}
			if (ImGui::MenuItem("Compile"))
			{
				if (m_selectedCompileOptionitem == 0)
					m_pascalCompiler->compilePascalToExe(m_mainEditor.GetFilepath(), false);
				else
					m_pascalCompiler->compilePascalToExe(m_optionalCustomFileToRunFromPath, false);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::BeginMenu("Editor Options"))
			{
				if (ImGui::BeginMenu("Text/Font"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Editor Style"))
			{
				if (ImGui::MenuItem("Dark palette"))
				{
					m_mainEditor.SetPalette(TextEditor::GetDarkPalette());
					for (TextEditor editor : m_additionalEditors)
						editor.SetPalette(TextEditor::GetDarkPalette());
					ThemeManager::SetApplicationDarkStyle();
				}
				if (ImGui::MenuItem("Light palette"))
				{
					m_mainEditor.SetPalette(TextEditor::GetLightPalette());
					for (TextEditor editor : m_additionalEditors)
						editor.SetPalette(TextEditor::GetLightPalette());
					ThemeManager::SetApplicationLightStyle();
				}
				if (ImGui::MenuItem("Retro blue palette"))
				{
					m_mainEditor.SetPalette(TextEditor::GetRetroBluePalette());
					for (TextEditor editor : m_additionalEditors)
						editor.SetPalette(TextEditor::GetRetroBluePalette());
					ThemeManager::SetApplicationDarkStyle();
				}
				if (ImGui::MenuItem("Pink palette"))
				{
					m_mainEditor.SetPalette(TextEditor::GetPinkyPalette());
					for (TextEditor editor : m_additionalEditors)
						editor.SetPalette(TextEditor::GetPinkyPalette());
					ThemeManager::SetApplicationCatpuccinMochaStyle();
				}
				if (ImGui::MenuItem("Dark Cherry"))
				{
					m_mainEditor.SetPalette(TextEditor::GetPinkyPalette());
					for (TextEditor editor : m_additionalEditors)
						editor.SetPalette(TextEditor::GetPinkyPalette());
					ThemeManager::SetApplicationDarkCherryStyle();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::Checkbox("Main", &m_showMain);
				ImGui::Checkbox("Console", &m_showConsole);
				ImGui::Checkbox("Autocompletion", &m_showAutocompletion);
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Templates"))
		{
			if (ImGui::MenuItem("New Program"))
			{
				LoadTempalte("new_file");
			}
			if (ImGui::MenuItem("If"))
			{
				LoadTempalte("if");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleColor(2);
}

void Window::ButtonSave()
{
	if (m_activeEditor->GetFilepath() != "")
	{
		if (SaveFile())
			m_activeEditor->MarkLinesSaved();
	}
	else
	{
		std::string savePath = FileDialogs::SaveFile("", *this);
		if (savePath != "")
		{
			m_activeEditor->SetFilepath(savePath);
			if (SaveFile())
				m_activeEditor->MarkLinesSaved();
		}
	}
}

void Window::ButtonOpen()
{
	std::string path = FileDialogs::OpenFile("", *this);
	m_mainEditor.SetFilepath(path);
	std::filesystem::path fsPath(path);
	std::string parentPath = fsPath.parent_path().string(); // Get the parent directory
	m_fileTreeRootNode = m_fileTree->CreateDirectryNodeTreeFromPath(parentPath);
	OpenFile(&m_mainEditor);
}

void Window::ButtonCreateNew()
{
	m_mainEditor.SetFilepath("");

	m_mainEditor.SelectAll();
	m_mainEditor.Delete();

	m_fileTreeRootNode.FullPath = "";

	LoadTempalte("new_file");
}

void Window::ButtonNewSourceFile()
{
	TextEditor additionalEditor;
	auto lang = TextEditor::LanguageDefinition::Pascal();
	additionalEditor.addOnCharTypedCallback([this](ImWchar aChar) { m_cInterface.UpdateLastInput(m_activeEditor); });
	additionalEditor.addOnCharTypedCallback([this](ImWchar aChar) { if (!strchr(autocompletionIgnoreKeys, aChar) && !m_activeEditor->IsBetweenQuotes()) this->AutoPredict(); });
	additionalEditor.addOnCharTypedCallback([this](ImWchar aChar) { if (!strchr(autocompletionIgnoreKeys, aChar) && !m_activeEditor->IsBetweenQuotes()) this->ActivateWordPredictionWindow(); });
	additionalEditor.SetLanguageDefinition(lang);
	m_additionalEditors.push_back(additionalEditor);
}

void Window::ButtonOpenNewSourceFile(std::string path)
{
	OutputDebugString(path.c_str());
	TextEditor additionalEditor;
	m_additionalEditors.push_back(additionalEditor);
	if (path == "")
		path = FileDialogs::OpenFile("", *this); //Open Dialog to select file to open when no direct file to open is given
	m_additionalEditors[m_additionalEditors.size() - 1].SetFilepath(path);
	OpenFile(&m_additionalEditors[m_additionalEditors.size() - 1]);
}

void Window::LoadTempalte(std::string templateName)
{
	std::string templatePath = (std::filesystem::current_path() / "Templates" / (templateName + ".txt")).string();

	std::fstream newfile;
	newfile.open(templatePath, std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{ //checking whether the file is open
		std::string tp;
		while (getline(newfile, tp))
		{ //read data from file object and put it into string.
			m_activeEditor->InsertText(tp + "\n", true);
		}
		newfile.close(); //close the file object.
	}
}

void Window::OpenFile(TextEditor* editor)
{
	editor->SelectAll();
	editor->Delete();

	std::fstream newfile;
	newfile.open(editor->GetFilepath(), std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{ //checking whether the file is open
		std::string tp;
		while (getline(newfile, tp))
		{ //read data from file object and put it into string.
			editor->InsertText(tp + "\n", false);
		}
		newfile.close(); //close the file object.
	}
}

bool Window::SaveFile()
{
	// Open the file in output mode, truncating it to overwrite existing content
	std::fstream file;
	file.open(m_activeEditor->GetFilepath(), std::ios::out | std::ios::trunc); // open file to write

	if (file.is_open())
	{
		std::string editorText = m_activeEditor->GetText();  // Get all text from the editor

		file << editorText;  // Write the text to the file

		file.close();  // Close the file when done
		return true;
	}
	else
	{
		// Handle file opening failure
		std::cerr << "Failed to open the file for saving." << std::endl;
		return false;
	}
}
