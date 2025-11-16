/*

------------------------------------------------------------------------------

Luma Engine - A Next-Generation Game Engine

Copyright (c) 2025 Nexel Games. All Rights Reserved.



This source code is part of the Luma Engine project developed by Nexel Games.

Use of this software is governed by the Luma Engine License Agreement.



Unauthorized copying of this file, via any medium, is strictly prohibited.

Distribution of source or binary forms, with or without modification, is

subject to the terms of the Luma Engine License.



For more information, visit: https://nexelgames.com/luma-engine

------------------------------------------------------------------------------

*/

#include "LGE/ui/ContentBrowser.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include "LGE/rendering/Texture.h"
#include "imgui.h"
#include <algorithm>
#include <memory>

namespace LGE {

ContentBrowser::ContentBrowser()
    : m_HistoryIndex(-1)
    , m_FilterType(EFileType::Unknown)
    , m_ShowDirectories(true)
    , m_ShowFiles(true)
    , m_ThumbnailSize(64.0f)
    , m_ViewMode(ViewMode::Icons)
{
    m_AssetsRoot = FileSystem::GetAssetsDirectory();
    m_CurrentDirectory = m_AssetsRoot;
    
    // Ensure assets directory exists
    if (!FileSystem::Exists(m_AssetsRoot)) {
        FileSystem::CreateDirectory(m_AssetsRoot);
    }
    
    // Ensure subdirectories exist
    std::string materialsDir = FileSystem::GetMaterialsDirectory();
    if (!FileSystem::Exists(materialsDir)) {
        FileSystem::CreateDirectory(materialsDir);
    }
    
    // Build initial directory tree
    RefreshDirectoryTree();
    RefreshAssets();
    
    // Load thumbnail images
    LoadThumbnails();
}

void ContentBrowser::SetProjectRoot(const std::string& root) {
    m_AssetsRoot = root;
    m_CurrentDirectory = root;
    RefreshDirectoryTree();
    RefreshAssets();
}

ContentBrowser::~ContentBrowser() {
}

void ContentBrowser::OnUIRender() {
    ImGui::Begin("Content Browser", nullptr);
    
    // Toolbar
    if (ImGui::Button("Refresh")) {
        RefreshDirectoryTree();
        RefreshAssets();
    }
    ImGui::SameLine();
    if (ImGui::Button("Back") && m_HistoryIndex > 0) {
        m_HistoryIndex--;
        m_CurrentDirectory = m_DirectoryHistory[m_HistoryIndex];
        RefreshAssets();
    }
    ImGui::SameLine();
    if (ImGui::Button("Forward") && m_HistoryIndex < static_cast<int>(m_DirectoryHistory.size()) - 1) {
        m_HistoryIndex++;
        m_CurrentDirectory = m_DirectoryHistory[m_HistoryIndex];
        RefreshAssets();
    }
    ImGui::SameLine();
    if (ImGui::Button("Up")) {
        NavigateUp();
    }
    ImGui::SameLine();
    
    // View mode toggle
    if (ImGui::Button(m_ViewMode == ViewMode::Icons ? "List" : "Icons")) {
        m_ViewMode = (m_ViewMode == ViewMode::Icons) ? ViewMode::List : ViewMode::Icons;
    }
    
    ImGui::Separator();
    
    // Current path display (breadcrumb)
    std::string relativePath = FileSystem::GetRelativePath(m_CurrentDirectory, m_AssetsRoot);
    if (relativePath.empty() || relativePath == ".") {
        ImGui::Text("Assets/");
    } else {
        ImGui::Text("Assets/%s/", relativePath.c_str());
    }
    
    ImGui::Separator();
    
    // Filter options
    ImGui::Text("Filter:");
    ImGui::SameLine();
    const char* filterItems[] = { "All", "Materials", "Textures", "Shaders", "Models", "Scenes" };
    static int currentFilter = 0;
    if (ImGui::Combo("##Filter", &currentFilter, filterItems, IM_ARRAYSIZE(filterItems))) {
        switch (currentFilter) {
            case 0: m_FilterType = EFileType::Unknown; break;
            case 1: m_FilterType = EFileType::Material; break;
            case 2: m_FilterType = EFileType::Texture; break;
            case 3: m_FilterType = EFileType::Shader; break;
            case 4: m_FilterType = EFileType::Model; break;
            case 5: m_FilterType = EFileType::Scene; break;
        }
        RefreshAssets();
    }
    
    ImGui::SameLine();
    ImGui::Text("Thumbnail Size:");
    ImGui::SameLine();
    ImGui::SliderFloat("##ThumbSize", &m_ThumbnailSize, 32.0f, 128.0f);
    
    ImGui::Separator();
    
    // Split view: folders on left, content on right
    float panelWidth = ImGui::GetContentRegionAvail().x;
    float folderPanelWidth = panelWidth * 0.25f;
    
    // Folder tree
    ImGui::BeginChild("FolderTree", ImVec2(folderPanelWidth, 0), true);
    RenderFolderTree();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Content area
    ImGui::BeginChild("ContentArea", ImVec2(0, 0), true);
    RenderContentArea();
    ImGui::EndChild();
    
    ImGui::End();
}

void ContentBrowser::RefreshAssets() {
    m_CurrentDirectoryEntries = FileSystem::ListDirectory(m_CurrentDirectory, false);
    
    // Sort: directories first, then files
    std::sort(m_CurrentDirectoryEntries.begin(), m_CurrentDirectoryEntries.end(),
        [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory > b.isDirectory; // Directories first
            }
            return a.name < b.name; // Alphabetical
        });
}

void ContentBrowser::SetCurrentDirectory(const std::string& path) {
    if (FileSystem::IsDirectory(path)) {
        m_CurrentDirectory = path;
        // Add to history
        if (m_HistoryIndex < static_cast<int>(m_DirectoryHistory.size()) - 1) {
            m_DirectoryHistory.erase(m_DirectoryHistory.begin() + m_HistoryIndex + 1, m_DirectoryHistory.end());
        }
        m_DirectoryHistory.push_back(m_CurrentDirectory);
        m_HistoryIndex = static_cast<int>(m_DirectoryHistory.size()) - 1;
        RefreshAssets();
    }
}

void ContentBrowser::RefreshDirectoryTree() {
    m_RootNode = BuildDirectoryTree(m_AssetsRoot);
}

std::shared_ptr<DirectoryNode> ContentBrowser::BuildDirectoryTree(const std::string& rootPath) {
    if (!FileSystem::Exists(rootPath) || !FileSystem::IsDirectory(rootPath)) {
        return nullptr;
    }
    
    auto root = std::make_shared<DirectoryNode>(rootPath, FileSystem::GetFileName(rootPath));
    root->isExpanded = true;  // Root is always expanded
    
    // Recursively build tree
    ScanDirectory(root);
    
    return root;
}

void ContentBrowser::ScanDirectory(std::shared_ptr<DirectoryNode> node) {
    if (node->hasScanned) return;
    
    try {
        auto entries = FileSystem::ListDirectory(node->path, false);
        
        for (const auto& entry : entries) {
            if (entry.isDirectory) {
                auto child = std::make_shared<DirectoryNode>(entry.path, entry.name);
                child->isExpanded = m_ExpandedPaths.find(entry.path) != m_ExpandedPaths.end();
                node->children.push_back(child);
                
                // Recursively scan if expanded
                if (child->isExpanded) {
                    ScanDirectory(child);
                }
            }
        }
        
        // Sort children alphabetically
        std::sort(node->children.begin(), node->children.end(),
            [](const std::shared_ptr<DirectoryNode>& a, const std::shared_ptr<DirectoryNode>& b) {
                return a->name < b->name;
            });
        
        node->hasScanned = true;
    } catch (const std::exception& e) {
        Log::Error("Failed to scan directory: " + node->path + " - " + e.what());
    }
}

void ContentBrowser::RenderFolderTree() {
    if (!m_RootNode) {
        ImGui::Text("No assets directory found");
        return;
    }
    
    RenderDirectoryNode(m_RootNode);
}

void ContentBrowser::RenderDirectoryNode(std::shared_ptr<DirectoryNode> node, int depth) {
    if (!node) return;
    
    // Indent based on depth
    for (int i = 0; i < depth; i++) {
        ImGui::Indent();
    }
    
    // Render folder icon and name
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (m_CurrentDirectory == node->path) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (node->children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    
    bool isOpen = ImGui::TreeNodeEx(node->name.c_str(), flags);
    
    // Handle click
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        SetCurrentDirectory(node->path);
    }
    
    // Handle double-click to navigate
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        OnDirectoryDoubleClicked(node->path);
    }
    
    if (isOpen) {
        if (!node->isExpanded) {
            node->isExpanded = true;
            m_ExpandedPaths.insert(node->path);
            ScanDirectory(node);  // Lazy load children
        }
        
        // Render children
        for (auto& child : node->children) {
            RenderDirectoryNode(child, depth + 1);
        }
        
        ImGui::TreePop();
    } else {
        if (node->isExpanded) {
            node->isExpanded = false;
            m_ExpandedPaths.erase(node->path);
        }
    }
    
    // Unindent
    for (int i = 0; i < depth; i++) {
        ImGui::Unindent();
    }
}

void ContentBrowser::RenderContentArea() {
    if (m_ViewMode == ViewMode::List) {
        // List view
        if (ImGui::BeginTable("ContentList", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();
            
            for (const auto& entry : m_CurrentDirectoryEntries) {
                // Apply filter
                if (m_FilterType != EFileType::Unknown && entry.fileType != m_FilterType && !entry.isDirectory) {
                    continue;
                }
                
                if (!m_ShowDirectories && entry.isDirectory) continue;
                if (!m_ShowFiles && !entry.isDirectory) continue;
                
                ImGui::TableNextRow();
                
                // Name
                ImGui::TableNextColumn();
                bool isSelected = (m_SelectedFilePath == entry.path);
                bool isEditing = (m_EditingFilePath == entry.path && entry.fileType == EFileType::Scene);
                
                if (isEditing) {
                    // Show input field for editing
                    char buffer[256];
                    strncpy_s(buffer, sizeof(buffer), m_EditingNameBuffer.c_str(), _TRUNCATE);
                    
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText("##EditSceneName", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                        // Apply rename
                        OnSceneNameEdited(entry, std::string(buffer));
                    }
                    
                    // Cancel editing if focus is lost
                    if (ImGui::IsItemDeactivatedAfterEdit() && !ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                        m_EditingFilePath.clear();
                        m_EditingNameBuffer.clear();
                    }
                } else {
                    std::string displayName = GetDisplayName(entry);
                    if (ImGui::Selectable(displayName.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                        OnFileSelected(entry);
                        
                        // If it's a scene and already selected, start editing on second click
                        if (isSelected && entry.fileType == EFileType::Scene) {
                            m_EditingFilePath = entry.path;
                            m_EditingNameBuffer = GetDisplayName(entry);
                        }
                    }
                    
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        if (entry.isDirectory) {
                            OnDirectoryDoubleClicked(entry.path);
                        } else {
                            OnFileDoubleClicked(entry);
                        }
                    }
                }
                
                // Type
                ImGui::TableNextColumn();
                if (entry.isDirectory) {
                    ImGui::Text("Folder");
                } else {
                    const char* typeName = "Unknown";
                    switch (entry.fileType) {
                        case EFileType::Material: typeName = "Material"; break;
                        case EFileType::Texture: typeName = "Texture"; break;
                        case EFileType::Shader: typeName = "Shader"; break;
                        case EFileType::Model: typeName = "Model"; break;
                        case EFileType::Scene: typeName = "Scene"; break;
                        default: break;
                    }
                    ImGui::Text("%s", typeName);
                }
                
                // Size
                ImGui::TableNextColumn();
                if (entry.isDirectory) {
                    ImGui::Text("-");
                } else {
                    // Format file size
                    if (entry.fileSize < 1024) {
                        ImGui::Text("%zu B", entry.fileSize);
                    } else if (entry.fileSize < 1024 * 1024) {
                        ImGui::Text("%.2f KB", entry.fileSize / 1024.0f);
                    } else {
                        ImGui::Text("%.2f MB", entry.fileSize / (1024.0f * 1024.0f));
                    }
                }
                
                // Modified (simplified - just show that it exists)
                ImGui::TableNextColumn();
                ImGui::Text("...");
            }
            
            ImGui::EndTable();
        }
    } else {
        // Icon/Thumbnail view
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float iconSize = m_ThumbnailSize;
        float padding = 10.0f;
        int itemsPerRow = static_cast<int>(panelWidth / (iconSize + padding));
        if (itemsPerRow < 1) itemsPerRow = 1;
        
        int itemIndex = 0;
        for (const auto& entry : m_CurrentDirectoryEntries) {
            // Apply filter
            if (m_FilterType != EFileType::Unknown && entry.fileType != m_FilterType && !entry.isDirectory) {
                continue;
            }
            
            if (!m_ShowDirectories && entry.isDirectory) continue;
            if (!m_ShowFiles && !entry.isDirectory) continue;
            
            // Start new row if needed
            if (itemIndex > 0 && itemIndex % itemsPerRow != 0) {
                ImGui::SameLine();
            }
            
            RenderAssetThumbnail(entry, iconSize);
            itemIndex++;
        }
        
        if (m_CurrentDirectoryEntries.empty()) {
            ImGui::Text("No assets found in this directory");
        }
    }
}

void ContentBrowser::RenderAssetThumbnail(const FileEntry& entry, float iconSize) {
    ImGui::BeginGroup();
    
    ImVec2 iconSizeVec(iconSize, iconSize);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Determine which thumbnail to use
    std::shared_ptr<Texture> thumbnail = nullptr;
    if (entry.isDirectory) {
        thumbnail = m_FolderThumbnail;
    } else if (entry.fileType == EFileType::Shader) {
        thumbnail = m_ShaderThumbnail;
    } else if (entry.fileType == EFileType::Script) {
        thumbnail = m_ScriptThumbnail;
    }
    
    // Draw background
    ImU32 bgColor = IM_COL32(40, 40, 40, 255);
    ImU32 borderColor = IM_COL32(100, 100, 100, 255);
    
    bool isSelected = (m_SelectedFilePath == entry.path);
    if (isSelected) {
        borderColor = IM_COL32(100, 150, 255, 255);
        drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + iconSizeVec.x, cursorPos.y + iconSizeVec.y), 
            IM_COL32(60, 90, 150, 100));
    }
    
    drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + iconSizeVec.x, cursorPos.y + iconSizeVec.y), bgColor);
    
    // Draw thumbnail image if available
    if (thumbnail && thumbnail->GetRendererID() != 0) {
        // For folders, apply light orange tint
        if (entry.isDirectory) {
            ImU32 tintColor = IM_COL32(255, 200, 120, 255); // Light orange
            drawList->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(thumbnail->GetRendererID())),
                cursorPos,
                ImVec2(cursorPos.x + iconSizeVec.x, cursorPos.y + iconSizeVec.y),
                ImVec2(0, 1),
                ImVec2(1, 0),
                tintColor
            );
        } else {
            ImGui::SetCursorScreenPos(cursorPos);
            ImGui::Image(
                reinterpret_cast<void*>(static_cast<intptr_t>(thumbnail->GetRendererID())),
                iconSizeVec,
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
        }
    } else {
        // Fallback: Draw colored rectangle with file type indicator
        if (entry.isDirectory) {
            bgColor = IM_COL32(200, 150, 100, 255);
            borderColor = IM_COL32(240, 180, 120, 255);
        } else {
            // Color code by file type
            switch (entry.fileType) {
                case EFileType::Material:
                    bgColor = IM_COL32(80, 60, 40, 255);
                    borderColor = IM_COL32(140, 100, 60, 255);
                    break;
                case EFileType::Texture:
                    bgColor = IM_COL32(40, 80, 60, 255);
                    borderColor = IM_COL32(60, 140, 100, 255);
                    break;
                case EFileType::Shader:
                    bgColor = IM_COL32(60, 40, 80, 255);
                    borderColor = IM_COL32(100, 60, 140, 255);
                    break;
                case EFileType::Model:
                    bgColor = IM_COL32(80, 80, 40, 255);
                    borderColor = IM_COL32(140, 140, 60, 255);
                    break;
                default:
                    break;
            }
        }
        
        drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + iconSizeVec.x, cursorPos.y + iconSizeVec.y), bgColor);
        
        // Draw file type indicator text
        float textSize = iconSize * 0.3f;
        ImVec2 textPos = ImVec2(cursorPos.x + iconSizeVec.x * 0.5f, cursorPos.y + iconSizeVec.y * 0.5f);
        const char* typeLabel = entry.isDirectory ? "DIR" : entry.extension.c_str();
        if (!entry.extension.empty() && !entry.isDirectory) {
            ImVec2 textSizeVec = ImGui::CalcTextSize(typeLabel);
            textPos.x -= textSizeVec.x * 0.5f;
            textPos.y -= textSizeVec.y * 0.5f;
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), typeLabel);
        }
    }
    
    // Draw border
    drawList->AddRect(cursorPos, ImVec2(cursorPos.x + iconSizeVec.x, cursorPos.y + iconSizeVec.y), borderColor, 0.0f, 0, 2.0f);
    
    // Draw text label below icon (or input field if editing)
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + iconSizeVec.y + 5));
    float labelWidth = iconSizeVec.x;
    
    bool isEditing = (m_EditingFilePath == entry.path && entry.fileType == EFileType::Scene);
    if (isEditing) {
        // Show input field for editing
        char buffer[256];
        strncpy_s(buffer, sizeof(buffer), m_EditingNameBuffer.c_str(), _TRUNCATE);
        
        ImGui::SetNextItemWidth(labelWidth);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##EditSceneNameIcon", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            // Apply rename
            OnSceneNameEdited(entry, std::string(buffer));
        }
        
        // Cancel editing if focus is lost
        if (ImGui::IsItemDeactivatedAfterEdit() && !ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            m_EditingFilePath.clear();
            m_EditingNameBuffer.clear();
        }
    } else {
        ImGui::PushTextWrapPos(cursorPos.x + labelWidth);
        std::string displayName = GetDisplayName(entry);
        ImGui::TextWrapped("%s", displayName.c_str());
        ImGui::PopTextWrapPos();
    }
    
    // Make entire area clickable (but not when editing)
    if (!isEditing) {
        ImGui::SetCursorScreenPos(cursorPos);
        std::string buttonId = "##" + entry.path;
        if (ImGui::InvisibleButton(buttonId.c_str(), ImVec2(iconSizeVec.x, iconSizeVec.y + 25))) {
            bool wasSelected = (m_SelectedFilePath == entry.path);
            OnFileSelected(entry);
            
            // If it's a scene and already selected, start editing on second click
            if (wasSelected && entry.fileType == EFileType::Scene) {
                m_EditingFilePath = entry.path;
                m_EditingNameBuffer = GetDisplayName(entry);
            }
        }
        
        // Handle double-click
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (entry.isDirectory) {
                OnDirectoryDoubleClicked(entry.path);
            } else {
                OnFileDoubleClicked(entry);
            }
        }
    }
    
    // Handle drag & drop
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", entry.path.c_str(), entry.path.length() + 1);
        std::string displayName = GetDisplayName(entry);
        ImGui::Text("%s", displayName.c_str());
        ImGui::EndDragDropSource();
    }
    
    ImGui::EndGroup();
}

void ContentBrowser::RenderAssetIcon(const FileEntry& entry) {
    RenderAssetThumbnail(entry, 64.0f);
}

void ContentBrowser::NavigateToDirectory(const std::string& path) {
    SetCurrentDirectory(path);
}

void ContentBrowser::NavigateUp() {
    std::string parent = FileSystem::GetDirectory(m_CurrentDirectory);
    if (!parent.empty() && FileSystem::IsDirectory(parent) && parent != m_CurrentDirectory) {
        SetCurrentDirectory(parent);
    }
}

void ContentBrowser::OnFileSelected(const FileEntry& entry) {
    m_SelectedFilePath = entry.path;
    Log::Info("Selected: " + entry.path);
}

void ContentBrowser::OnSceneNameEdited(const FileEntry& entry, const std::string& newName) {
    if (newName.empty() || newName == GetDisplayName(entry)) {
        // Cancel editing
        m_EditingFilePath.clear();
        m_EditingNameBuffer.clear();
        return;
    }
    
    // Validate new name (no invalid characters)
    std::string sanitizedName = newName;
    // Remove invalid file name characters
    const std::string invalidChars = "<>:\"/\\|?*";
    for (char& c : sanitizedName) {
        if (invalidChars.find(c) != std::string::npos) {
            c = '_';
        }
    }
    
    if (sanitizedName.empty()) {
        sanitizedName = "Scene";
    }
    
    // Build new file path
    std::string oldPath = entry.path;
    std::string directory = FileSystem::GetDirectory(oldPath);
    std::string newFileName = sanitizedName + ".lscene";
    std::string newPath = FileSystem::JoinPath(directory, newFileName);
    
    // Check if new file already exists
    if (FileSystem::Exists(newPath) && newPath != oldPath) {
        Log::Error("A file with that name already exists: " + newPath);
        m_EditingFilePath.clear();
        m_EditingNameBuffer.clear();
        return;
    }
    
    // Rename the file
    if (FileSystem::MoveFile(oldPath, newPath)) {
        Log::Info("Renamed scene: " + oldPath + " -> " + newPath);
        
        // Notify callback
        if (m_OnSceneRenamed) {
            m_OnSceneRenamed(oldPath, newPath);
        }
        
        // Refresh the directory
        RefreshAssets();
        
        // Clear editing state
        m_EditingFilePath.clear();
        m_EditingNameBuffer.clear();
        
        // Update selection to new path
        m_SelectedFilePath = newPath;
    } else {
        Log::Error("Failed to rename scene file: " + oldPath);
        m_EditingFilePath.clear();
        m_EditingNameBuffer.clear();
    }
}

void ContentBrowser::OnFileDoubleClicked(const FileEntry& entry) {
    // Open file in appropriate editor based on type
    switch (entry.fileType) {
        case EFileType::Material:
            Log::Info("Open material in Material Editor: " + entry.path);
            // TODO: Open Material Editor with this material
            break;
        case EFileType::Texture:
            Log::Info("Open texture in Texture Importer: " + entry.path);
            // TODO: Open Texture Importer with this texture
            break;
        case EFileType::Shader:
            Log::Info("Open shader: " + entry.path);
            // TODO: Open shader in code editor
            break;
        case EFileType::Scene:
            Log::Info("Loading scene: " + entry.path);
            // Load the scene file
            if (m_OnSceneOpened) {
                m_OnSceneOpened(entry.path);
            }
            break;
        default:
            Log::Info("Open file: " + entry.path);
            break;
    }
}

void ContentBrowser::OnDirectoryDoubleClicked(const std::string& path) {
    SetCurrentDirectory(path);
}

std::shared_ptr<DirectoryNode> ContentBrowser::FindNodeByPath(std::shared_ptr<DirectoryNode> root, const std::string& path) {
    if (!root || root->path == path) {
        return root;
    }
    
    for (auto& child : root->children) {
        auto found = FindNodeByPath(child, path);
        if (found) {
            return found;
        }
    }
    
    return nullptr;
}

void ContentBrowser::LoadThumbnails() {
    // Try to find thumbnail files in common locations
    std::vector<std::string> searchPaths = {
        "build/temp_icons/imgs/Icons/Folders/Folder_Base_256x.png",
        "build/unreal_icons/imgs/Icons/Folders/Folder_Base_256x.png",
        "unreal-engine-editor-icons/imgs/Icons/Folders/Folder_Base_256x.png",
        "assets/icons/Folder_Base_256x.png"
    };
    
    // Load folder thumbnail
    for (const auto& path : searchPaths) {
        if (FileSystem::Exists(path)) {
            m_FolderThumbnail = std::make_shared<Texture>();
            TextureSpec spec;
            spec.filepath = path;
            spec.gammaCorrected = true;
            spec.minFilter = TextureFilter::Linear;
            spec.magFilter = TextureFilter::Linear;
            spec.wrapS = TextureWrap::ClampToEdge;
            spec.wrapT = TextureWrap::ClampToEdge;
            if (m_FolderThumbnail->Load(spec)) {
                Log::Info("Loaded folder thumbnail: " + path);
                break;
            }
            m_FolderThumbnail.reset();
        }
    }
    
    // Load script thumbnail
    std::vector<std::string> scriptPaths = {
        "build/temp_icons/imgs/GameProjectDialog/feature_code.png",
        "build/unreal_icons/imgs/GameProjectDialog/feature_code.png",
        "unreal-engine-editor-icons/imgs/GameProjectDialog/feature_code.png",
        "assets/icons/feature_code.png"
    };
    
    for (const auto& path : scriptPaths) {
        if (FileSystem::Exists(path)) {
            m_ScriptThumbnail = std::make_shared<Texture>();
            TextureSpec spec;
            spec.filepath = path;
            spec.gammaCorrected = true;
            spec.minFilter = TextureFilter::Linear;
            spec.magFilter = TextureFilter::Linear;
            spec.wrapS = TextureWrap::ClampToEdge;
            spec.wrapT = TextureWrap::ClampToEdge;
            if (m_ScriptThumbnail->Load(spec)) {
                Log::Info("Loaded script thumbnail: " + path);
                break;
            }
            m_ScriptThumbnail.reset();
        }
    }
    
    // Load shader thumbnail
    std::vector<std::string> shaderPaths = {
        "build/assets/icons/Shader_Thumbnail.png",
        "assets/icons/Shader_Thumbnail.png"
    };
    
    for (const auto& path : shaderPaths) {
        if (FileSystem::Exists(path)) {
            m_ShaderThumbnail = std::make_shared<Texture>();
            TextureSpec spec;
            spec.filepath = path;
            spec.gammaCorrected = true;
            spec.minFilter = TextureFilter::Linear;
            spec.magFilter = TextureFilter::Linear;
            spec.wrapS = TextureWrap::ClampToEdge;
            spec.wrapT = TextureWrap::ClampToEdge;
            if (m_ShaderThumbnail->Load(spec)) {
                Log::Info("Loaded shader thumbnail: " + path);
                break;
            }
            m_ShaderThumbnail.reset();
        }
    }
}

std::string ContentBrowser::GetDisplayName(const FileEntry& entry) const {
    // For scene files, strip the .lscene extension
    if (entry.fileType == EFileType::Scene && entry.extension == ".lscene") {
        std::string name = entry.name;
        // Remove .lscene extension
        size_t extPos = name.rfind(".lscene");
        if (extPos != std::string::npos) {
            return name.substr(0, extPos);
        }
    }
    // For all other files, return the name as-is
    return entry.name;
}

} // namespace LGE

