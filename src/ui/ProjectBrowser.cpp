/*

------------------------------------------------------------------------------

 Luma Engine - Project Browser

 Rewritten: 2025 Nexel Games

------------------------------------------------------------------------------

*/



#include "LGE/ui/ProjectBrowser.h"

#include "LGE/core/project/Project.h"

#include "LGE/core/filesystem/FileSystem.h"

#include "LGE/core/Log.h"

#include "LGE/rendering/Texture.h"



#include <imgui.h>

#include <algorithm>

#include <ctime>

#include <iomanip>

#include <sstream>

#include <limits>

#include <filesystem>

#ifdef _WIN32

#include <windows.h>

// Undefine Windows macros that conflict with FileSystem methods

#ifdef CreateDirectory

#undef CreateDirectory

#endif

#else

#include <unistd.h>

#endif



namespace LGE {



// ==========================================================================

// INTERNAL UI HELPERS

// ==========================================================================

namespace UI {



// Clickable card (Template cards)

bool Card(const char* id, ImTextureID texture, const ImVec2& size, bool selected) {

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImVec2 max = ImVec2(pos.x + size.x, pos.y + size.y);



    ImU32 bg = selected ? IM_COL32(50, 90, 150, 255) : IM_COL32(42, 42, 42, 255);

    ImU32 border = selected ? IM_COL32(66, 150, 250, 255) : IM_COL32(60, 60, 60, 255);



    dl->AddRectFilled(pos, max, bg, 0.0f);

    dl->AddRect(pos, max, border, 0.0f, 0, selected ? 2.0f : 1.0f);



    // Image (thumbnail fills the entire card)

    if (texture && texture != (ImTextureID)0) {

        // Thumbnail fills the entire card area with color tint

        // Light orange tint for folder icon

        ImU32 tintColor = IM_COL32(255, 200, 120, 255); // Light orange

        dl->AddImage(texture, pos, max, ImVec2(0,0), ImVec2(1,1), tintColor);

    } else {

        // Placeholder if no texture

        ImU32 placeholderBg = IM_COL32(30, 30, 30, 255);

        dl->AddRectFilled(

            ImVec2(pos.x + 8, pos.y + 8),

            ImVec2(pos.x + size.x - 8, pos.y + size.y - 40),

            placeholderBg

        );

    }



    // Label

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 8, max.y - 24));

    ImGui::TextUnformatted(id);



    // Click

    ImGui::SetCursorScreenPos(pos);

    return ImGui::InvisibleButton(id, size);

}



// List row (Projects + Recent)

bool Row(const std::string& id, bool selected, float height = 55.0f) {

    ImVec2 s(ImGui::GetContentRegionAvail().x, height);

    return ImGui::Selectable(id.c_str(), selected, 0, s);

}



// Category row with thumbnail

bool CategoryRow(const std::string& id, ImTextureID thumbnail, bool selected, float height = 60.0f) {

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImVec2 size(ImGui::GetContentRegionAvail().x, height);

    ImVec2 max = ImVec2(pos.x + size.x, pos.y + size.y);



    // Background

    ImU32 bg = selected ? IM_COL32(40, 80, 140, 255) : IM_COL32(30, 30, 30, 255);

    dl->AddRectFilled(pos, max, bg);



    // Thumbnail (60x60)

    ImVec2 thumbSize(60.0f, 60.0f);

    ImVec2 thumbPos = pos;

    ImVec2 thumbMax = ImVec2(thumbPos.x + thumbSize.x, thumbPos.y + thumbSize.y);



    if (thumbnail) {

        ImGui::SetCursorScreenPos(thumbPos);

        ImGui::Image(thumbnail, thumbSize, ImVec2(0,0), ImVec2(1,1));

    } else {

        // Placeholder

        dl->AddRectFilled(thumbPos, thumbMax, IM_COL32(40, 40, 40, 255));

    }



    // Text

    ImGui::SetCursorScreenPos(ImVec2(thumbPos.x + thumbSize.x + 12.0f, thumbPos.y + 20.0f));

    ImVec4 textColor = selected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Text, textColor);

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0,0,0,0));

    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0));

    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0));



    bool clicked = ImGui::Selectable(id.c_str(), selected, 0, ImVec2(size.x - thumbSize.x - 12.0f, size.y));



    ImGui::PopStyleColor(4);



    // Selection border

    if (selected) {

        dl->AddRect(pos, max, IM_COL32(66, 150, 250, 255), 0.0f, 0, 2.0f);

    }



    return clicked;

}



} // namespace UI





// ==========================================================================

// CONSTRUCTOR

// ==========================================================================

ProjectBrowser::ProjectBrowser()

    : m_ViewMode(ViewMode::Projects),

      m_SelectedCategory(Category::Recent),

      m_SelectedTemplateIndex(-1),

      m_SelectedProjectIndex(-1),

      m_IncludeStarterContent(false)

{

    m_NewProjectNameBuffer[0] = 0;

    m_NewProjectPathBuffer[0] = 0;



#ifdef _WIN32

    const char* profile = getenv("USERPROFILE");

    if (profile)

        m_ProjectsDirectory = std::string(profile) + "\\Documents\\LGE\\Projects";

    else

        m_ProjectsDirectory = "Projects";

#else

    const char* home = getenv("HOME");

    if (home)

        m_ProjectsDirectory = std::string(home) + "/Documents/LGE/Projects";

    else

        m_ProjectsDirectory = "Projects";

#endif



    strncpy_s(m_NewProjectPathBuffer, m_ProjectsDirectory.c_str(), sizeof(m_NewProjectPathBuffer)-1);



    if (!FileSystem::Exists(m_ProjectsDirectory))

        FileSystem::CreateDirectory(m_ProjectsDirectory);



    RefreshProjectList();

    LoadThumbnails();

}



ProjectBrowser::~ProjectBrowser() {

}





// ==========================================================================

// MAIN WINDOW

// ==========================================================================

bool ProjectBrowser::OnUIRender() {

    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);

    ImGui::SetNextWindowSize(vp->Size);



    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.18f,.18f,.18f,1));



    ImGuiWindowFlags flags =

        ImGuiWindowFlags_NoTitleBar |

        ImGuiWindowFlags_NoMove |

        ImGuiWindowFlags_NoResize |

        ImGuiWindowFlags_NoDocking |

        ImGuiWindowFlags_NoCollapse;



    bool open = true;

    ImGui::Begin("ProjectBrowser", &open, flags);



    float leftWidth = 240.0f;

    float bottomHeight = 70.0f;



    // Left

    ImGui::BeginChild("LeftPanel", ImVec2(leftWidth, -bottomHeight), true);

    RenderLeftPanel();

    ImGui::EndChild();



    ImGui::SameLine();



    // Right

    ImGui::BeginChild("RightPanel", ImVec2(0, -bottomHeight), true);

    RenderRightPanel();

    ImGui::EndChild();



    // Bottom Bar

    ImGui::BeginChild("BottomBar", ImVec2(0, bottomHeight), true);

    RenderBottomBar();

    ImGui::EndChild();



    ImGui::End();

    ImGui::PopStyleColor();



    return m_SelectedProject != nullptr;

}





// ==========================================================================

// LEFT PANEL

// ==========================================================================

void ProjectBrowser::RenderLeftPanel() {

    ImGui::Spacing();

    ImGui::TextColored(ImVec4(.7f,.7f,.7f,1), "CATEGORIES");

    ImGui::Separator();

    ImGui::Spacing();

    RenderCategories();

}





// ==========================================================================

// RECENT PROJECTS

// ==========================================================================

void ProjectBrowser::RenderRecentProjects() {

    int limit = (m_Projects.size() < 5) ? static_cast<int>(m_Projects.size()) : 5;

    if (!limit) {

        ImGui::TextColored(ImVec4(.5f,.5f,.5f,1), "No recent projects");

        return;

    }



    for (int i = 0; i < limit; i++) {

        bool selected = (m_SelectedProjectIndex == i);

        if (UI::Row(m_Projects[i].name + "##recent" + std::to_string(i), selected)) {

            m_SelectedProjectIndex = i;

            m_ViewMode = ViewMode::Projects;

        }

    }

}





// ==========================================================================

// CATEGORIES

// ==========================================================================

void ProjectBrowser::RenderCategories() {

    // Recent category
    bool recentSelected = (m_SelectedCategory == Category::Recent);
    ImTextureID recentTex = (ImTextureID)0;
    if (m_FolderThumbnail && m_FolderThumbnail->GetRendererID() != 0) {
        recentTex = (ImTextureID)(intptr_t)m_FolderThumbnail->GetRendererID();
    }
    if (UI::CategoryRow("RECENT##cat0", recentTex, recentSelected)) {
        m_SelectedCategory = Category::Recent;
        m_ViewMode = ViewMode::Projects;
        m_SelectedTemplateIndex = -1;
    }

    // Games category
    bool gamesSelected = (m_SelectedCategory == Category::Games);
    ImTextureID gamesTex = (ImTextureID)0;
    if (m_GamesThumbnail && m_GamesThumbnail->GetRendererID() != 0) {
        gamesTex = (ImTextureID)(intptr_t)m_GamesThumbnail->GetRendererID();
    }
    if (UI::CategoryRow("GAMES##cat1", gamesTex, gamesSelected)) {
        m_SelectedCategory = Category::Games;
        m_ViewMode = ViewMode::Templates;
        m_SelectedTemplateIndex = -1;
    }

    // Learning category
    bool learningSelected = (m_SelectedCategory == Category::Learning);
    ImTextureID learningTex = (ImTextureID)0;
    if (m_FolderThumbnail && m_FolderThumbnail->GetRendererID() != 0) {
        learningTex = (ImTextureID)(intptr_t)m_FolderThumbnail->GetRendererID();
    }
    if (UI::CategoryRow("LEARNING##cat2", learningTex, learningSelected)) {
        m_SelectedCategory = Category::Learning;
        m_ViewMode = ViewMode::Templates;
        m_SelectedTemplateIndex = -1;
    }

}





// ==========================================================================

// RIGHT PANEL

// ==========================================================================

void ProjectBrowser::RenderRightPanel() {

    float detailsWidth = 320.0f;



    ImGui::BeginChild("MainContent", ImVec2(-detailsWidth, 0), true);



    if (m_ViewMode == ViewMode::Templates)

        RenderProjectTemplates();

    else

        RenderProjectList();



    ImGui::EndChild();



    ImGui::SameLine();



    ImGui::BeginChild("Details", ImVec2(detailsWidth,0), true);



    if (m_ViewMode == ViewMode::Templates)

        RenderTemplateDetails();

    else

        RenderProjectDetails();



    ImGui::EndChild();

}





// ==========================================================================

// TEMPLATE GRID

// ==========================================================================

void ProjectBrowser::RenderProjectTemplates() {

    const char* names[] = {

        "Blank"

    };



    std::shared_ptr<Texture> thumbs[] = {

        m_BlankThumbnail

    };



    // Card size matches thumbnail size exactly

    float card = 128.0f; // Card size matches thumbnail size

    float spacing = 16.0f;

    float panelWidth = ImGui::GetContentRegionAvail().x;



    // Calculate number of columns that fit

    int cols = static_cast<int>((panelWidth + spacing) / (card + spacing));

    if (cols < 1) cols = 1;



    // Start with some padding

    ImGui::SetCursorPos(ImVec2(spacing, spacing));



    for (int i = 0; i < 1; i++) {

        ImTextureID tex = (ImTextureID)0;

        if (thumbs[i] && thumbs[i]->GetRendererID() != 0) {

            tex = (ImTextureID)(intptr_t)thumbs[i]->GetRendererID();

            Log::Info("Template " + std::string(names[i]) + " has texture ID: " + std::to_string(thumbs[i]->GetRendererID()));

        } else {

            Log::Error("Template " + std::string(names[i]) + " has no valid texture");

        }



        if (UI::Card(names[i], tex, ImVec2(card, card), m_SelectedTemplateIndex == i))

            m_SelectedTemplateIndex = i;

    }

}





// ==========================================================================

// PROJECT LIST

// ==========================================================================

void ProjectBrowser::RenderProjectList() {

    if (m_Projects.empty()) {
        ImGui::TextColored(ImVec4(.5f,.5f,.5f,1), "No projects found.");
        return;
    }

    // Card size matches thumbnail size exactly
    float card = 128.0f; // Card size matches thumbnail size
    float spacing = 16.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;

    // Calculate number of columns that fit
    int cols = static_cast<int>((panelWidth + spacing) / (card + spacing));
    if (cols < 1) cols = 1;

    // Start with some padding
    ImGui::SetCursorPos(ImVec2(spacing, spacing));

    for (int i = 0; i < m_Projects.size(); i++) {
        bool selected = (m_SelectedProjectIndex == i);

        // Use project thumbnail if available, otherwise use folder thumbnail
        ImTextureID tex = (ImTextureID)0;
        if (m_FolderThumbnail && m_FolderThumbnail->GetRendererID() != 0) {
            tex = (ImTextureID)(intptr_t)m_FolderThumbnail->GetRendererID();
        }

        // Calculate position for grid layout
        int row = i / cols;
        int col = i % cols;
        float x = spacing + col * (card + spacing);
        float y = spacing + row * (card + spacing);
        ImGui::SetCursorPos(ImVec2(x, y));

        if (UI::Card(m_Projects[i].name.c_str(), tex, ImVec2(card, card), selected)) {
            m_SelectedProjectIndex = i;
            m_ViewMode = ViewMode::Projects;
        }
    }

}





// ==========================================================================

// TEMPLATE DETAILS

// ==========================================================================

void ProjectBrowser::RenderTemplateDetails() {

    if (m_SelectedTemplateIndex < 0) {

        ImGui::TextColored(ImVec4(.5f,.5f,.5f,1), "Select a template to view details.");

        return;

    }



    const char* titles[] = {

        "Blank"

    };



    const char* desc[] = {

        "A blank project with no starting assets."

    };



    ImGui::TextColored(ImVec4(1,1,1,1), "%s", titles[m_SelectedTemplateIndex]);

    ImGui::Separator();

    ImGui::Spacing();



    // Preview image

    if (m_BlankPreview && m_BlankPreview->GetRendererID() != 0) {

        float previewWidth = ImGui::GetContentRegionAvail().x - 20.0f;

        float aspectRatio = 16.0f / 9.0f; // Common preview aspect ratio

        float previewHeight = previewWidth / aspectRatio;

        ImGui::Image(

            (ImTextureID)(intptr_t)m_BlankPreview->GetRendererID(),

            ImVec2(previewWidth, previewHeight),

            ImVec2(0, 0),

            ImVec2(1, 1)

        );

        ImGui::Spacing();

    } else {

        Log::Error("Blank preview texture not loaded or invalid");

    }



    ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 30);

    ImGui::TextColored(ImVec4(.75f,.75f,.75f,1), "%s", desc[m_SelectedTemplateIndex]);

    ImGui::PopTextWrapPos();



    ImGui::Spacing();

    ImGui::Checkbox("Include Starter Content", &m_IncludeStarterContent);

}





// ==========================================================================

// PROJECT DETAILS PANEL

// ==========================================================================

void ProjectBrowser::RenderProjectDetails() {

    if (m_SelectedProjectIndex < 0 || m_SelectedProjectIndex >= m_Projects.size()) {

        ImGui::TextColored(ImVec4(.5f,.5f,.5f,1), "Select a project to view details.");

        return;

    }



    const auto& p = m_Projects[m_SelectedProjectIndex];



    ImGui::TextColored(ImVec4(1,1,1,1), "%s", p.name.c_str());

    ImGui::Separator();

    ImGui::Spacing();



    ImGui::Text("Path:"); ImGui::SameLine();

    ImGui::TextColored(ImVec4(.8f,.8f,.8f,1), "%s", p.path.c_str());



    ImGui::Text("Version:"); ImGui::SameLine();

    ImGui::TextColored(ImVec4(.8f,.8f,.8f,1), "%s", p.version.c_str());



    ImGui::Text("Last Modified:"); ImGui::SameLine();

    ImGui::TextColored(ImVec4(.8f,.8f,.8f,1), "%s", FormatDate(p.lastModified).c_str());



    if (!p.description.empty()) {

        ImGui::Spacing();

        ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 30);

        ImGui::TextColored(ImVec4(.75f,.75f,.75f,1), "%s", p.description.c_str());

        ImGui::PopTextWrapPos();

    }



    ImGui::Spacing();

    if (ImGui::Button("Open Project", ImVec2(150, 32)))

        OpenProject(p);

}





// ==========================================================================

// BOTTOM BAR

// ==========================================================================

void ProjectBrowser::RenderBottomBar() {

    ImGui::Text("Location:");

    ImGui::SameLine();

    ImGui::SetNextItemWidth(300);

    ImGui::InputText("##loc", m_NewProjectPathBuffer, sizeof(m_NewProjectPathBuffer));



    ImGui::SameLine();

    if (ImGui::Button("Browse"))

        Log::Info("Folder selection not implemented.");



    ImGui::SameLine();

    ImGui::Text("Name:");

    ImGui::SameLine();

    ImGui::SetNextItemWidth(200);

    ImGui::InputText("##name", m_NewProjectNameBuffer, sizeof(m_NewProjectNameBuffer));



    bool canProceed =

        (m_ViewMode == ViewMode::Templates && m_SelectedTemplateIndex >= 0 && strlen(m_NewProjectNameBuffer) > 0) ||

        (m_ViewMode == ViewMode::Projects && m_SelectedProjectIndex >= 0);



    ImGui::SameLine(ImGui::GetWindowWidth() - 260);



    if (ImGui::Button("Cancel", ImVec2(110, 30))) {}



    ImGui::SameLine();



    if (!canProceed) ImGui::BeginDisabled();



    if (ImGui::Button(m_ViewMode == ViewMode::Templates ? "Create" : "Open", ImVec2(110,30))) {

        if (m_ViewMode == ViewMode::Templates)

            CreateNewProject(m_NewProjectNameBuffer, m_NewProjectPathBuffer);

        else

            OpenProject(m_Projects[m_SelectedProjectIndex]);

    }



    if (!canProceed) ImGui::EndDisabled();

}





// ==========================================================================

// PROJECT OPEN / CREATE

// ==========================================================================

void ProjectBrowser::OpenProject(const ProjectBrowserEntry& e) {

    auto p = Project::Load(e.projectFilePath);

    if (p && p->IsLoaded()) {

        m_SelectedProject = p;

        if (m_OnProjectOpened) m_OnProjectOpened(p);

    } else {
        Log::Error("Failed to load project: " + e.projectFilePath);
    }

}



void ProjectBrowser::CreateNewProject(const std::string& name, const std::string& path) {

    // Validate project name is not empty
    if (name.empty()) {
        Log::Error("Project name cannot be empty");
        return;
    }

    // Check if a project with the same name already exists in the target location
    std::string full = FileSystem::JoinPath(path, name);
    std::string projectFilePath = FileSystem::JoinPath(full, name + ".luma");
    
    // Check if the project directory already exists
    if (FileSystem::Exists(full)) {
        // Check if it contains a project file with the same name
        if (FileSystem::Exists(projectFilePath)) {
            Log::Error("A project with the name \"" + name + "\" already exists at " + path);
            return;
        }
        // Directory exists but no project file - this is okay, we can create the project
    }

    auto p = Project::CreateNew(full, name);



    if (p && p->IsLoaded()) {

        m_SelectedProject = p;

        if (m_OnProjectOpened) m_OnProjectOpened(p);

        RefreshProjectList();

    } else {
        Log::Error("Failed to create project: " + name);
    }

}





// ==========================================================================

// UTILITIES

// ==========================================================================

void ProjectBrowser::RefreshProjectList() {

    m_Projects.clear();

    ScanForProjects();

}



void ProjectBrowser::ScanForProjects() {

    auto list = FileSystem::ListDirectory(m_ProjectsDirectory, true);



    for (auto& e : list) {

        if (!e.isDirectory && e.extension == ".luma") {

            ProjectBrowserEntry pb;

            pb.projectFilePath = e.path;

            pb.path = FileSystem::GetDirectory(e.path);

            pb.name = FileSystem::GetFileNameWithoutExtension(e.path);



            auto p = std::make_shared<Project>();

            if (p->Load(e.path)) {

                auto info = p->GetInfo();

                pb.description = info.description;

                pb.version = info.version;

                pb.lastModified = info.lastModified;

                pb.isValid = true;

            }



            m_Projects.push_back(pb);

        }

    }



    std::sort(m_Projects.begin(), m_Projects.end(),

              [](auto& a, auto& b) { return a.lastModified > b.lastModified; });

}



std::string ProjectBrowser::FormatDate(uint64_t ts) const {

    if (!ts) return "Unknown";



    std::time_t t = ts;

    std::tm* tm = std::localtime(&t);



    std::stringstream ss;

    ss << std::put_time(tm, "%Y-%m-%d %H:%M");

    return ss.str();

}





// ==========================================================================

// THUMBNAILS

// ==========================================================================

void ProjectBrowser::LoadThumbnails() {

    auto load = [&](std::shared_ptr<Texture>& tex, const std::vector<std::string>& paths) {

        for (auto& p : paths) {

            // Try multiple path locations (similar to ReadFile)

            std::vector<std::string> pathsToTry;

            pathsToTry.push_back(p); // Relative to current directory

            // Try relative to executable directory (go up to project root)

            #ifdef _WIN32

            char exePath[MAX_PATH];

            ::GetModuleFileNameA(NULL, exePath, MAX_PATH);

            std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

            std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();

            pathsToTry.push_back((projectRoot / p).string());

            #else

            char exePath[1024];

            ssize_t count = readlink("/proc/self/exe", exePath, 1024);

            if (count != -1) {

                std::filesystem::path exeDir = std::filesystem::path(std::string(exePath, count)).parent_path();

                std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();

                pathsToTry.push_back((projectRoot / p).string());

            }

            #endif



            for (auto& tryPath : pathsToTry) {

                if (FileSystem::Exists(tryPath)) {

                    tex = std::make_shared<Texture>();

                    TextureSpec s;

                    s.filepath = tryPath;

                    s.gammaCorrected = true;

                    s.minFilter = TextureFilter::Linear;

                    s.magFilter = TextureFilter::Linear;

                    s.wrapS = TextureWrap::ClampToEdge;

                    s.wrapT = TextureWrap::ClampToEdge;

                    s.generateMipmaps = false;

                    if (tex->Load(s)) {

                        Log::Info("Loaded thumbnail: " + tryPath);

                        return;

                    } else {

                        Log::Error("Failed to load thumbnail: " + tryPath);

                        tex.reset();

                    }

                }

            }

        }

    };



    load(m_FolderThumbnail, {

        "assets/icons/Folder_Base_256x.png",

        "build/temp_icons/imgs/Icons/Folders/Folder_Base_256x.png"

    });



    load(m_BlankThumbnail, {

        "assets/icons/Folder_Base_256x.png",

        "build/temp_icons/imgs/Icons/Folders/Folder_Base_256x.png"

    });



    load(m_BlankPreview, {

        "assets/icons/blank_project_preview.png",

        "build/temp_icons/imgs/GameProjectDialog/blank_project_preview.png"

    });

    load(m_FirstPersonThumbnail, {"assets/First_Person_Thumbnail.png"});

    load(m_ThirdPersonThumbnail, {"assets/Third_Person_Thumbnail.png"});

    load(m_TopDownThumbnail,     {"assets/Top_Down_Thumbnail.png"});

    load(m_VehicleThumbnail,     {"assets/Vehicle_Thumbnail.png"});

    load(m_VRThumbnail,          {"assets/VR_Thumbnail.png"});



    // Category thumbnails

    load(m_GamesThumbnail,       {"assets/Games_Thumbnail.png", "build/assets/Games_Thumbnail.png"});

}



} // namespace LGE
