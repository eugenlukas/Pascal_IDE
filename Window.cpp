#include "Window.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "FileDialogs.h"

void Window::CreateMainWindow()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	m_Window = glfwCreateWindow(1920, 1080, "STINKENDE SCHMUTZ IDE: Version 0.69420", NULL, NULL);

	// Error check if the window fails to create
	if (m_Window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	// Maximize the window
	glfwMaximizeWindow(m_Window);

	// Introduce the window into the current context
	glfwMakeContextCurrent(m_Window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 800, 800);

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	auto lang = TextEditor::LanguageDefinition::Pascal();
	m_Editor.SetLanguageDefinition(lang);

	// Main while loop
	while (!glfwWindowShouldClose(m_Window))
	{
		Render();
	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// Delete window before ending the program
	glfwDestroyWindow(m_Window);
	// Terminate GLFW before ending the program
	glfwTerminate();
}

void Window::Render()
{
	auto cpos = m_Editor.GetCursorPosition();

	// Specify the color of the background
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);

	// Tell OpenGL a new frame is about to begin
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static float popupTimer = 0.0f;
	static bool showPopup = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Create new"))
			{
				m_pascalFilePath = "";
				m_fileToEdit = "NONE";

				m_Editor.SelectAll();
				m_Editor.Delete();
			}
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				m_pascalFilePath = FileDialogs::OpenFile("", *this);
				m_fileToEdit = m_pascalFilePath.c_str();
				OpenFile();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				if (m_pascalFilePath != "")
				{
					if (SaveFile())
					{
						showPopup = true; // Show the popup when the Save button is clicked
						popupTimer = 0.0f; // Reset the timer
					}
				}
				else
				{
					m_pascalFilePath = FileDialogs::SaveFile("", *this);
					if (SaveFile())
					{
						showPopup = true; // Show the popup when the Save button is clicked
						popupTimer = 0.0f; // Reset the timer
					}
					m_fileToEdit = m_pascalFilePath.c_str();
				}
			}
			if (ImGui::MenuItem("Save as.."))
			{
				m_pascalFilePath = FileDialogs::SaveFile("Pascal File (*pas)\0.pas\0", *this);
				if (SaveFile())
				{
					showPopup = true; // Show the popup when the Save button is clicked
					popupTimer = 0.0f; // Reset the timer
				}
				m_fileToEdit = m_pascalFilePath.c_str();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run/Compile"))
		{
			if (ImGui::MenuItem("Compile and Run", "F9"))
			{
				m_PascalCompiler->compilePascalToExe(m_pascalFilePath, true);
			}
			if (ImGui::MenuItem("Run"))
			{
				m_PascalCompiler->runCompiledExe(m_pascalFilePath);
			}
			if (ImGui::MenuItem("Compile"))
			{
				m_PascalCompiler->compilePascalToExe(m_pascalFilePath, false);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
			{
				m_Editor.SetPalette(TextEditor::GetDarkPalette());
			}
			if (ImGui::MenuItem("Light palette"))
			{
				m_Editor.SetPalette(TextEditor::GetLightPalette());
			}
			if (ImGui::MenuItem("Retro blue palette"))
			{
				m_Editor.SetPalette(TextEditor::GetRetroBluePalette());
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// ImGUI window creation
	ImGui::Begin("Editor");
	// Text that appears in the window
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, m_Editor.GetTotalLines(),
		m_Editor.IsOverwrite() ? "Ovr" : "Ins",
		m_Editor.CanUndo() ? "*" : " ",
		m_Editor.GetLanguageDefinition().mName.c_str(), m_fileToEdit);
	m_Editor.Render("Editor");
	// Ends the window

	if (showPopup)
	{
		// Update the timer
		popupTimer += ImGui::GetIO().DeltaTime;

		// Show the popup
		ImGui::OpenPopup("Saved!");

		if (popupTimer > 1.0f) // Wait for 1 second
		{
			showPopup = false;  // Hide the popup after 1 second
			popupTimer = 0.0f;  // Reset the timer
		}
	}

	if (ImGui::BeginPopup("Saved!"))
	{
		ImGui::Text("Saved!");
		ImGui::EndPopup();
	}
	ImGui::End();

	// Renders the ImGUI elements
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap the back buffer with the front buffer
	glfwSwapBuffers(m_Window);
	// Take care of all GLFW events
	glfwPollEvents();
}

void Window::OpenFile()
{
	m_Editor.SelectAll();
	m_Editor.Delete();

	std::fstream newfile;
	newfile.open(m_pascalFilePath, std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{ //checking whether the file is open
		std::string tp;
		while (getline(newfile, tp))
		{ //read data from file object and put it into string.
			m_Editor.InsertText(tp + "\n");
		}
		newfile.close(); //close the file object.
	}
}

bool Window::SaveFile()
{
	// Open the file in output mode, truncating it to overwrite existing content
	std::fstream file;
	file.open(m_pascalFilePath, std::ios::out | std::ios::trunc); // open file to write

	if (file.is_open())
	{
		// Retrieve the content from the editor (assuming InsertText is similar to appending text)
		std::string editorText = m_Editor.GetText();  // Get all text from the editor

		file << editorText;  // Write the text to the file

		file.close();  // Close the file when done
		return true;
	}
	else
	{
		// Handle file opening failure (optional, can add user alert or logging)
		std::cerr << "Failed to open the file for saving." << std::endl;
		return false;
	}
}

const char* Window::m_fileToEdit = "NONE";
