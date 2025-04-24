#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

struct DirectoryNode
{
	std::string FullPath;
	std::string FileName;
	std::vector<DirectoryNode> Children;
	bool IsDirectory;

	// Default constructors and assignment operators
	DirectoryNode() = default;
	DirectoryNode(const DirectoryNode&) = default;
	DirectoryNode& operator=(const DirectoryNode&) = default;
};

class FileTree
{
public:
	FileTree();
	~FileTree();

	using OnCharTypedCallback = std::function<void(std::string)>;

	void addOnCharTypedCallback(OnCharTypedCallback callback)
	{
		openFileCallback = callback;
	}

	void Render(const DirectoryNode& parentNode, bool first = false); //For Render the parent node is the root node
	DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath);

private:
	void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator);

	OnCharTypedCallback openFileCallback;
};