#include <iostream>
#include <fstream>
#include "PascalCompiler.h"
#include <thread>
#include "FileLogger.h"
#include <regex>

PROCESS_INFORMATION PascalCompiler::g_compileExeProcess = {};

PascalCompiler::PascalCompiler()
{
}

PascalCompiler::~PascalCompiler()
{
}

bool PascalCompiler::compilePascalToExe(const std::string& pascalFile, bool autoRun)
{
    if (pascalFile == "")
        return false;

    //(Re-)Generate fpc config paths
    GenerateFpcConfig();

    //Reset old error
    m_errors.clear();

    // Convert Pascal file path to an absolute path
    std::string pascalFilePath = std::filesystem::absolute(pascalFile).string();

    // Get the current directory path
    std::string currentPath = std::filesystem::current_path().string();

    // Construct include paths first (place the most generic ones first)
    std::string includePaths = "-Fi" + currentPath + "\\Lazarus\\components\\lazutils";
    includePaths.append(" -Fi" + currentPath + "\\Lazarus\\lcl\\include");

    // Construct unit paths after the include paths
    std::string unitPaths = "-Fu" + currentPath + "\\Lazarus\\lcl";
    unitPaths.append(" -Fu" + currentPath + "\\Lazarus\\lcl\\widgetset");
    unitPaths.append(" -Fu" + currentPath + "\\Lazarus\\lcl\\forms");
    unitPaths.append(" -Fu" + currentPath + "\\Lazarus\\lcl\\interfaces\\win32");
    unitPaths.append(" -Fu" + currentPath + "\\Lazarus\\components\\lazutils");

    // Combine the include and unit paths, prioritizing include paths
    std::string paths = includePaths + " " + unitPaths;

    // Construct the command with PowerShell-compatible format
    std::string command = "powershell -Command \"& '" + fpcPath + "' " + paths + " '" + pascalFilePath + "' 2>&1\"";
    OutputDebugString((command + "\n").c_str());
    LOG("Try running command: " + command);


    // Run the command and capture the result
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        OutputDebugString("Error running PowerShell command.");
        LOG("Error: could not run PowerShell command");
        return false;
    }

    // Read the output and errors from the pipe
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        m_errors.push_back(buffer);
    }

    // Close the pipe
    int result = _pclose(pipe);

    // Check the result of the compilation
    if (result != 0)
    {
        OutputDebugString("Compilation failed.");
        LOG("Compilation failed");
        return false;
    }

    LOG("Successfuly runned compile program");

    if (autoRun)
    {
        size_t dotPos = pascalFilePath.rfind('.');
        pascalFilePath.replace(dotPos, std::string::npos, ".exe");
        std::string openExeCommand = pascalFilePath;
        
        //Open compiled program
        STARTUPINFO si = { sizeof(STARTUPINFO) };
        if (!CreateProcessA(openExeCommand.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &g_compileExeProcess))
        {
            OutputDebugString("Failed to start compiled exe. Error: " + GetLastError());
            LOG("Error: Failed to start compiled exe. Messege:\n" + GetLastError());
        }
    }

    return true;
}

void PascalCompiler::runCompiledExe(const std::string& pascalFile)
{
    std::string pascalFilePath = std::filesystem::absolute(pascalFile).string();

    size_t dotPos = pascalFilePath.rfind('.');
    pascalFilePath.replace(dotPos, std::string::npos, ".exe");
    std::string openExeCommand = pascalFilePath;

    //Open compiled program
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    if (!CreateProcessA(openExeCommand.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &g_compileExeProcess))
    {
        OutputDebugString("Failed to start compiled exe. Error: " + GetLastError());
        LOG("Error: Failed to start compiled exe. Messege:\n" + GetLastError());
    }
}

bool PascalCompiler::IsErrorLine(const std::string& line)
{
    //ErrorKind::Global; // Default

    // Format 1: file(line,col) Type: Message
    size_t parenStart = line.find('(');
    size_t parenEnd = line.find(')', parenStart);
    size_t colon = line.find(':', parenEnd);

    if (parenStart != std::string::npos && parenEnd != std::string::npos && colon != std::string::npos)
    {
        std::string file = line.substr(0, parenStart);
        std::string loc = line.substr(parenStart + 1, parenEnd - parenStart - 1);
        size_t comma = loc.find(',');

        //ErrorKind::Regular;
        return true;
    }

    // Format 2: file(line) Type: Message (no column)
    size_t dot = line.find(".pas(");
    if (dot != std::string::npos)
    {
        size_t paren = line.find('(', dot);
        size_t endParen = line.find(')', paren);
        size_t colon2 = line.find(':', endParen);

        if (paren != std::string::npos && endParen != std::string::npos && colon2 != std::string::npos)
        {
            //ErrorKind::FileOnly;
            return true;
        }
    }

    // Format 3: Global Error (no file)
    if (line.find("Fatal:") == 0 || line.find("Error:") == 0)
    {
        //ErrorKind::Global;
        return true;
    }

    return false;
}

CompilationError PascalCompiler::GetErrorInLine(const std::string& line)
{
    std::regex pattern(R"((.+)\((\d+),(\d+)\)\s+(Error|Fatal):\s+(.*))");
    std::smatch match;

    std::string filename = match[1];
    int lineno = std::stoi(match[2]);
    int column = std::stoi(match[3]);
    std::string type = match[4];
    std::string message = match[5];

    return CompilationError(filename, lineno, column, type, message);
}

void PascalCompiler::GenerateFormFiles(const std::vector<ImGuiCanvasElement>& elements, const std::string& formName)
{
    // Create form file
    std::ofstream lfmFile(formName + ".lfm");
    if (!lfmFile.is_open())
    {
        std::cerr << "Error: Could not create " << formName << ".lfm file." << std::endl;
        return;
    }

    // Start writing the form file
    lfmFile << "object MainAppForm: T" << formName << "\n";
    lfmFile << "  Left = 192\n  Height = 400\n  Top = 106\n  Width = 600\n  Caption = '" << formName << "'\n";

    // Iterate through objects to add them to the .lfm
    for (const auto& obj : elements)
    {
        lfmFile << "  object " << obj.name.c_str() << ": " << elementTypeToString(obj.getType()) << "\n";
        lfmFile << "    Left = " << static_cast<int>(std::round(obj.m_center.position.x)) << "\n";
        lfmFile << "    Top = " << static_cast<int>(std::round(obj.m_center.position.y)) << "\n";
        lfmFile << "    Width = " << obj.size.x << "\n";
        lfmFile << "    Height = " << obj.size.y << "\n";
        for (ElementAttribute attribute : obj.attributes)
        {
            lfmFile << "    " << attribute.attributeName << " = '" << attribute.attributeValue << "'\n";
        }
        lfmFile << "  end\n";
    }
    lfmFile << "end\n";
    lfmFile.close();

    // Create .pas file
    std::ofstream pasFile(formName + ".pas");
    if (!pasFile.is_open())
    {
        std::cerr << "Error: Could not create " << formName << ".pas file." << std::endl;
        return;
    }

    // Write the Pascal unit code
    pasFile << "unit " << formName << ";\n\n";
    pasFile << "{$mode objfpc}{$H+}\n\n";
    pasFile << "interface\n\n";
    pasFile << "uses\n  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, interfaces;\n\n";
    pasFile << "type\n";
    pasFile << "  T" << formName << " = class(TForm)\n";

    // Add declarations for each object as fields in the class
    for (const auto& obj : elements)
    {
        pasFile << "    " << obj.name << ": " << elementTypeToString(obj.getType()) << ";\n";
    }

    pasFile << "  private\n";
    pasFile << "    { private declarations }\n";
    pasFile << "  public\n";
    pasFile << "    { public declarations }\n";
    pasFile << "  end;\n\n";

    pasFile << "var\n";
    pasFile << "  MainAppForm: T" << formName << ";\n\n";
    pasFile << "implementation\n\n";
    pasFile << "{$R *.lfm}\n\n";
    pasFile << "end.\n";

    pasFile.close();

    // Create .pas project file
    std::ofstream projectFile("Project1.pas");
    if (!projectFile.is_open())
    {
        std::cerr << "Error: Could not create " << formName << ".pas file." << std::endl;
        return;
    }

    projectFile << "program Project1;\n\n";
    projectFile << "uses\n";
    projectFile << "  Forms, ";
    projectFile << formName << " in '" << formName << ".pas' {T" << formName << "};\n\n";
    projectFile << "begin\n";
    projectFile << "  Application.Initialize;\n";
    projectFile << "  Application.CreateForm(T" << formName << ", MainAppForm);\n";
    projectFile << "  Application.Run;\n";
    projectFile << "end.";

    projectFile.close();
}

void PascalCompiler::GenerateFpcConfig()
{
    std::string fpcFolderPath = currentPath + "\\FPC\\3.2.2";
    std::string fpcConfigPath = currentPath + "\\FPC\\3.2.2\\bin\\i386-win32\\fpc.cfg";

    std::ofstream cfgFile(fpcConfigPath);
    if (!cfgFile.is_open())
    {
        LOG("Error: Could not create fpc.cfg file.");
        return;
    }

    std::string file = R""""(
# ----------------------
# Defines (preprocessor)
# ----------------------

#
# nested #IFNDEF, #IFDEF, #ENDIF, #ELSE, #DEFINE, #UNDEF are allowed
#
# -d is the same as #DEFINE
# -u is the same as #UNDEF
#

#
# Some examples (for switches see below, and the -? helppages)
#
# Try compiling with the -dRELEASE or -dDEBUG on the commandline
#

# For a release compile with optimizes and strip debuginfo
#IFDEF RELEASE
  -O2
  -Xs
  #WRITE Compiling Release Version
#ENDIF

# For a debug version compile with debuginfo and all codegeneration checks on
#IFDEF DEBUG
  -gl
  -Crtoi
  #WRITE Compiling Debug Version
#ELSE
  # Strip debuginfo from the executable if not in debug mode
  -Xs
#ENDIF

# assembling
#ifdef darwin
# use pipes instead of temporary files for assembling
-ap
#endif

# ----------------
# Parsing switches
# ----------------

# Pascal language mode
#      -Mfpc      free pascal dialect (default)
#      -Mobjfpc   switch some Delphi 2 extensions on
#      -Mdelphi   tries to be Delphi compatible
#      -Mtp       tries to be TP/BP 7.0 compatible
#      -Mgpc      tries to be gpc compatible
#      -Mmacpas   tries to be compatible to the macintosh pascal dialects
#
# Turn on Object Pascal extensions by default
#-Mobjfpc

# Assembler reader mode
#      -Rdefault  use default assembler
#      -Ratt      read AT&T style assembler
#      -Rintel    read Intel style assembler
#
# All assembler blocks are AT&T styled by default
#-Ratt

# Semantic checking
#      -S2        same as -Mobjfpc
#      -Sc        supports operators like C (*=,+=,/= and -=)
#      -Sa        include assertion code.
#      -Sd        same as -Mdelphi
#      -Se<x>     error options. <x> is a combination of the following:
#         <n> : compiler stops after <n> errors (default is 1)
#         w   : compiler stops also after warnings
#         n   : compiler stops also after notes
#         h   : compiler stops also after hints
#      -Sg        allow LABEL and GOTO
#      -Sh        Use ansistrings
#      -Si        support C++ styled INLINE
#      -Sk        load fpcylix unit
#      -SI<x>     set interface style to <x>
#         -SIcom    COM compatible interface (default)
#         -SIcorba  CORBA compatible interface
#      -Sm        support macros like C (global)
#      -So        same as -Mtp
#      -Sp        same as -Mgpc
#      -Ss        constructor name must be init (destructor must be done)
#      -Sx        enable exception keywords (default in Delphi/ObjFPC modes)
#
# Allow goto, inline, C-operators, C-vars
-Sgic

# ---------------
# Code generation
# ---------------

# Uncomment the next line if you always want static/dynamic units by default
# (can be overruled with -CD, -CS at the commandline)
#-CS
#-CD

# Set the default heapsize to 8Mb
#-Ch8000000

# Set default codegeneration checks (iocheck, overflow, range, stack)
#-Ci
#-Co
#-Cr
#-Ct

# Optimizer switches
# -Os        generate smaller code
# -Oa=N      set alignment to N
# -O1        level 1 optimizations (quick optimizations, debuggable)
# -O2        level 2 optimizations (-O1 + optimizations which make debugging more difficult)
# -O3        level 3 optimizations (-O2 + optimizations which also may make the program slower rather than faster)
# -Oo<x>     switch on optimalization x. See fpc -i for possible values
# -OoNO<x>   switch off optimalization x. See fpc -i for possible values
# -Op<x>     set target cpu for optimizing, see fpc -i for possible values

#ifdef darwin
#ifdef cpui386
-Cppentiumm
-Oppentiumm
#endif
#endif

# -----------------------
# Set Filenames and Paths
# -----------------------

# Both slashes and backslashes are allowed in paths

# path to the messagefile, not necessary anymore but can be used to override
# the default language
#-Fr{fpcFolderPath}/msg/errore.msg
#-Fr{fpcFolderPath}/msg/errorn.msg
#-Fr{fpcFolderPath}/msg/errores.msg
#-Fr{fpcFolderPath}/msg/errord.msg
#-Fr{fpcFolderPath}/msg/errorr.msg

# search path for unicode binary files (FPC 2.x does not know this switch)
#ifndef VER2
-FM{fpcFolderPath}/unicode/
#endif

# Search for $fpctarget/$fpcsubarch-$fpcmemorymodel/ subdirectory first
# for i8086 CPU
#ifdef cpui8086
-Fu{fpcFolderPath}/units/$fpctarget/$fpcsubarch-$fpcmemorymodel
-Fu{fpcFolderPath}/units/$fpctarget/$fpcsubarch-$fpcmemorymodel/*
-Fu{fpcFolderPath}/units/$fpctarget/$fpcsubarch-$fpcmemorymodel/rtl
#endif

# searchpath for units and other system dependent things
-Fu{fpcFolderPath}/units/$fpctarget
-Fu{fpcFolderPath}/units/$fpctarget/*
-Fu{fpcFolderPath}/units/$fpctarget/rtl

#IFDEF FPCAPACHE_1_3
-Fu{fpcFolderPath}/units/$fpctarget/httpd13/
#ELSE
#IFDEF FPCAPACHE_2_0
-Fu{fpcFolderPath}/units/$fpctarget/httpd20
#ELSE
-Fu{fpcFolderPath}/units/$fpctarget/httpd22
#ENDIF
#ENDIF

# searchpath for fppkg user-specific packages
-Fu$LOCAL_APPDATA\FreePascal\fppkg/units/$FPCTARGET/*

# searchpath for tools
-FD{fpcFolderPath}/bin/$FPCTARGET

# path to the gcclib


# searchpath for libraries
#-FlC:\FPC\3.2.2/lib
#-Fl/lib;/usr/lib
-Fl{fpcFolderPath}/lib/$FPCTARGET

#IFNDEF CPUI386
#IFNDEF CPUAMD64
#DEFINE NEEDCROSSBINUTILS
#ENDIF
#ENDIF

#IFNDEF Win32
#DEFINE NEEDCROSSBINUTILS
#ENDIF

# never need cross-prefix when targeting the JVM
# (no native compiler, always cross-compiling)
#ifdef cpujvm
#undef NEEDCROSSBINUTILS
#endif

# for android cross-prefix is set by compiler
#ifdef android
#undef NEEDCROSSBINUTILS
#endif

# never need cross-prefix when targeting the i8086
# (no native compiler, always cross-compiling)
#ifdef cpui8086
#undef NEEDCROSSBINUTILS
#endif

# never need cross-prefix when targeting the i8086
# (no native compiler, always cross-compiling)
#ifdef cpujvm
#undef NEEDCROSSBINUTILS
#endif

# binutils prefix for cross compiling
#IFDEF FPC_CROSSCOMPILING
#IFDEF NEEDCROSSBINUTILS
  -XP$FPCTARGET-
#ENDIF
#ENDIF


# -------------
# Linking
# -------------

# generate always debugging information for GDB (slows down the compiling
# process)
#      -gc        generate checks for pointers
#      -gd        use dbx
#      -gg        use gsym
#      -gh        use heap trace unit (for memory leak debugging)
#      -gl        use line info unit to show more info for backtraces
#      -gv        generates programs tracable with valgrind
#      -gw        generate dwarf debugging info
#
# Enable debuginfo and use the line info unit by default
#-gl

# always pass an option to the linker
#-k-s

# Always use smartlinking on i8086, because the system unit exceeds the 64kb
# code limit
#ifdef cpui8086
-CX
-XX
#endif

# -------------
# Miscellaneous
# -------------

# Write always a nice FPC logo ;)
-l

# Verbosity
#      e : Show errors (default)       d : Show debug info
#      w : Show warnings               u : Show unit info
#      n : Show notes                  t : Show tried/used files
#      h : Show hints                  s : Show time stamps
#      i : Show general info           q : Show message numbers
#      l : Show linenumbers            c : Show conditionals
#      a : Show everything             0 : Show nothing (except errors)
#      b : Write file names messages   r : Rhide/GCC compatibility mode
#          with full path              x : Executable info (Win32 only)
#      v : write fpcdebug.txt with     p : Write tree.log with parse tree
#          lots of debugging info
#
# Display Info, Warnings and Notes
-viwn
# If you don't want so much verbosity use
#-vw
)"""";

    ReplacePlaceholder(file, "{fpcFolderPath}", fpcFolderPath);

    cfgFile.clear();
    cfgFile << file;
    cfgFile.close();
}

bool PascalCompiler::IsCompiledProgramRunning()
{
    DWORD exitCode;
    if (GetExitCodeProcess(g_compileExeProcess.hProcess, &exitCode))
    {
        if (exitCode == STILL_ACTIVE)
            return true;
        else
            return false;
    }
}

void PascalCompiler::StopCompiledProgram()
{
    if (IsCompiledProgramRunning()) //Check if program is at all running
    {
        TerminateProcess(g_compileExeProcess.hProcess, 1);
        CloseHandle(g_compileExeProcess.hThread);
    }
}

std::vector<std::string> PascalCompiler::GetErrors()
{
    return m_errors;
}

void PascalCompiler::ReplacePlaceholder(std::string& source, const std::string& placeholder, const std::string& value)
{
    size_t pos = 0;
    while ((pos = source.find(placeholder, pos)) != std::string::npos)
    {
        source.replace(pos, placeholder.length(), value);
        pos += value.length();
    }
}
