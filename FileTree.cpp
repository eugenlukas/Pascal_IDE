#include "FileTree.h"
#include <imgui.h>
#include <Windows.h>

FileTree::FileTree()
{
}

FileTree::~FileTree()
{
}

void FileTree::Render(const DirectoryNode& parentNode, bool first)
{
	ImGui::PushID(&parentNode);
	if (parentNode.IsDirectory)
	{
		if (first)
			ImGui::SetNextItemOpen(true);
		if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
		{
			for (const DirectoryNode& childNode : parentNode.Children)
				Render(childNode);
			ImGui::TreePop();
		}
	}
	else
	{
		if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			// ... handle file click interaction
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				openFileCallback(parentNode.FullPath);
		}
	}
	ImGui::PopID();
}

void FileTree::RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{
	for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryNode& childNode = parentNode.Children.emplace_back();
		childNode.FullPath = entry.path().string();
		childNode.FileName = entry.path().filename().string();
		if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
		{
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
		}
	}

	auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

DirectoryNode FileTree::CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
{
	DirectoryNode rootNode;
	rootNode.FullPath = rootPath.string();
	rootNode.FileName = rootPath.filename().string();
	if (rootNode.IsDirectory = std::filesystem::is_directory(rootPath); rootNode.IsDirectory)
		RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

	return rootNode;
}
