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

#include "LGE/ui/TextureImporter.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/rendering/Texture.h"
#include "imgui.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace LGE {

TextureImporter::TextureImporter()
    : m_TextureManager(nullptr)
    , m_Visible(false)
    , m_HasTexture(false)
{
    m_CurrentSpec = TextureSpec();
}

TextureImporter::~TextureImporter() {
}

void TextureImporter::OnUIRender() {
    if (!m_Visible) return;
    
    ImGui::Begin("Texture Importer", &m_Visible);
    
    // Drag & drop area
    HandleDragDrop();
    
    if (m_HasTexture) {
        ImGui::Separator();
        
        // Texture preview
        RenderTexturePreview();
        
        ImGui::Separator();
        
        // Texture metadata
        RenderTextureMetadata();
        
        ImGui::Separator();
        
        // Texture settings
        RenderTextureSettings();
        
        ImGui::Separator();
        
        // Save button
        if (ImGui::Button("Save Texture Asset", ImVec2(-1, 0))) {
            if (!m_CurrentFilePath.empty()) {
                // Generate .texture asset path
                std::filesystem::path path(m_CurrentFilePath);
                std::string assetPath = path.replace_extension(".texture").string();
                SaveTextureAsset(assetPath, m_CurrentSpec);
            }
        }
    }
    
    ImGui::End();
}

void TextureImporter::HandleDragDrop() {
    ImGui::Text("Drag & Drop texture file here");
    ImGui::Spacing();
    
    // Create drop target
    ImVec2 canvasSize = ImVec2(ImGui::GetContentRegionAvail().x, 100.0f);
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##DropTarget", canvasSize);
    
    // Draw drop area
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    bool isHovered = ImGui::IsItemHovered();
    ImU32 color = isHovered ? IM_COL32(100, 150, 200, 255) : IM_COL32(50, 50, 50, 255);
    drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), color, 0.0f, 0, 2.0f);
    
    // Center text
    ImVec2 textSize = ImGui::CalcTextSize("Drop texture file here");
    ImVec2 textPos = ImVec2(canvasPos.x + (canvasSize.x - textSize.x) * 0.5f, 
                            canvasPos.y + (canvasSize.y - textSize.y) * 0.5f);
    drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), "Drop texture file here");
    
    // Handle drop
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* filepath = static_cast<const char*>(payload->Data);
            m_CurrentFilePath = std::string(filepath);
            
            // Load texture
            if (m_TextureManager) {
                m_CurrentSpec.filepath = m_CurrentFilePath;
                m_PreviewTexture = m_TextureManager->GetOrLoad(m_CurrentSpec);
                m_HasTexture = (m_PreviewTexture != nullptr);
                
                if (m_HasTexture) {
                    // Update spec from loaded texture
                    m_CurrentSpec.gammaCorrected = m_PreviewTexture->IsGammaCorrected();
                    m_CurrentSpec.minFilter = m_PreviewTexture->GetMinFilter();
                    m_CurrentSpec.magFilter = m_PreviewTexture->GetMagFilter();
                    m_CurrentSpec.wrapS = m_PreviewTexture->GetWrapS();
                    m_CurrentSpec.wrapT = m_PreviewTexture->GetWrapT();
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    // Also allow file path input
    ImGui::Spacing();
    ImGui::Text("Or enter file path:");
    static char filepathBuffer[512] = "";
    ImGui::InputText("##FilePath", filepathBuffer, sizeof(filepathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        if (strlen(filepathBuffer) > 0) {
            m_CurrentFilePath = std::string(filepathBuffer);
            if (m_TextureManager) {
                m_CurrentSpec.filepath = m_CurrentFilePath;
                m_PreviewTexture = m_TextureManager->GetOrLoad(m_CurrentSpec);
                m_HasTexture = (m_PreviewTexture != nullptr);
                
                if (m_HasTexture) {
                    m_CurrentSpec.gammaCorrected = m_PreviewTexture->IsGammaCorrected();
                    m_CurrentSpec.minFilter = m_PreviewTexture->GetMinFilter();
                    m_CurrentSpec.magFilter = m_PreviewTexture->GetMagFilter();
                    m_CurrentSpec.wrapS = m_PreviewTexture->GetWrapS();
                    m_CurrentSpec.wrapT = m_PreviewTexture->GetWrapT();
                }
            }
        }
    }
}

void TextureImporter::RenderTexturePreview() {
    if (!m_PreviewTexture || m_PreviewTexture->GetRendererID() == 0) {
        ImGui::Text("No texture loaded");
        return;
    }
    
    ImGui::Text("Preview:");
    
    // Calculate preview size (max 256x256, maintain aspect ratio)
    int texWidth = m_PreviewTexture->GetWidth();
    int texHeight = m_PreviewTexture->GetHeight();
    float aspectRatio = static_cast<float>(texWidth) / static_cast<float>(texHeight);
    
    float previewWidth = 256.0f;
    float previewHeight = 256.0f / aspectRatio;
    if (previewHeight > 256.0f) {
        previewHeight = 256.0f;
        previewWidth = 256.0f * aspectRatio;
    }
    
    ImGui::Image(
        reinterpret_cast<void*>(static_cast<intptr_t>(m_PreviewTexture->GetRendererID())),
        ImVec2(previewWidth, previewHeight),
        ImVec2(0, 1), ImVec2(1, 0)  // Flip vertically for OpenGL
    );
}

void TextureImporter::RenderTextureMetadata() {
    if (!m_PreviewTexture) return;
    
    ImGui::Text("Metadata:");
    ImGui::Text("File: %s", m_CurrentFilePath.c_str());
    ImGui::Text("Size: %d x %d", m_PreviewTexture->GetWidth(), m_PreviewTexture->GetHeight());
    ImGui::Text("Format: %s", m_PreviewTexture->IsHDR() ? "HDR" : "LDR");
    ImGui::Text("Type: %s", m_PreviewTexture->IsCubemap() ? "Cubemap" : "2D");
}

void TextureImporter::RenderTextureSettings() {
    if (!m_PreviewTexture) return;
    
    ImGui::Text("Settings:");
    
    // Gamma correction
    bool gammaCorrected = m_CurrentSpec.gammaCorrected;
    if (ImGui::Checkbox("Gamma Corrected (SRGB)", &gammaCorrected)) {
        m_CurrentSpec.gammaCorrected = gammaCorrected;
        // Reload texture with new settings
        if (m_TextureManager) {
            m_TextureManager->Unload(m_CurrentFilePath);
            m_PreviewTexture = m_TextureManager->GetOrLoad(m_CurrentSpec);
        }
    }
    
    // Min filter
    const char* minFilterItems[] = { "Nearest", "Linear", "Nearest Mipmap Nearest", 
                                     "Linear Mipmap Nearest", "Nearest Mipmap Linear", 
                                     "Linear Mipmap Linear" };
    int currentMinFilter = static_cast<int>(m_CurrentSpec.minFilter);
    if (ImGui::Combo("Min Filter", &currentMinFilter, minFilterItems, IM_ARRAYSIZE(minFilterItems))) {
        m_CurrentSpec.minFilter = static_cast<TextureFilter>(currentMinFilter);
        if (m_PreviewTexture) {
            m_PreviewTexture->SetFilter(m_CurrentSpec.minFilter, m_CurrentSpec.magFilter);
        }
    }
    
    // Mag filter (only Nearest or Linear)
    const char* magFilterItems[] = { "Nearest", "Linear" };
    int currentMagFilter = static_cast<int>(m_CurrentSpec.magFilter);
    if (currentMagFilter > 1) currentMagFilter = 1; // Clamp to valid range
    if (ImGui::Combo("Mag Filter", &currentMagFilter, magFilterItems, 2)) {
        m_CurrentSpec.magFilter = static_cast<TextureFilter>(currentMagFilter);
        if (m_PreviewTexture) {
            m_PreviewTexture->SetFilter(m_CurrentSpec.minFilter, m_CurrentSpec.magFilter);
        }
    }
    
    // Wrap S
    const char* wrapItems[] = { "Repeat", "Clamp To Edge", "Clamp To Border", "Mirrored Repeat" };
    int currentWrapS = static_cast<int>(m_CurrentSpec.wrapS);
    if (ImGui::Combo("Wrap S", &currentWrapS, wrapItems, IM_ARRAYSIZE(wrapItems))) {
        m_CurrentSpec.wrapS = static_cast<TextureWrap>(currentWrapS);
        if (m_PreviewTexture) {
            m_PreviewTexture->SetWrap(m_CurrentSpec.wrapS, m_CurrentSpec.wrapT);
        }
    }
    
    // Wrap T
    int currentWrapT = static_cast<int>(m_CurrentSpec.wrapT);
    if (ImGui::Combo("Wrap T", &currentWrapT, wrapItems, IM_ARRAYSIZE(wrapItems))) {
        m_CurrentSpec.wrapT = static_cast<TextureWrap>(currentWrapT);
        if (m_PreviewTexture) {
            m_PreviewTexture->SetWrap(m_CurrentSpec.wrapS, m_CurrentSpec.wrapT);
        }
    }
    
    // Generate mipmaps
    ImGui::Checkbox("Generate Mipmaps", &m_CurrentSpec.generateMipmaps);
}

void TextureImporter::SaveTextureAsset(const std::string& filepath, const TextureSpec& spec) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Log::Error("Failed to create texture asset file: " + filepath);
        return;
    }
    
    // Simple JSON writer (manual, no external dependency)
    file << "{\n";
    file << "  \"path\": \"" << spec.filepath << "\",\n";
    file << "  \"type\": \"" << (spec.filepath.find(".hdr") != std::string::npos || 
                                  spec.filepath.find(".exr") != std::string::npos ? "HDR" : "2D") << "\",\n";
    
    // Wrap mode
    const char* wrapStr = "Repeat";
    switch (spec.wrapS) {
        case TextureWrap::ClampToEdge: wrapStr = "ClampToEdge"; break;
        case TextureWrap::ClampToBorder: wrapStr = "ClampToBorder"; break;
        case TextureWrap::MirroredRepeat: wrapStr = "MirroredRepeat"; break;
        default: wrapStr = "Repeat"; break;
    }
    file << "  \"wrap\": \"" << wrapStr << "\",\n";
    
    // Filter mode
    const char* filterStr = "Linear";
    switch (spec.minFilter) {
        case TextureFilter::Nearest: filterStr = "Nearest"; break;
        default: filterStr = "Linear"; break;
    }
    file << "  \"filter\": \"" << filterStr << "\",\n";
    
    file << "  \"gammaCorrected\": " << (spec.gammaCorrected ? "true" : "false") << ",\n";
    file << "  \"generateMipmaps\": " << (spec.generateMipmaps ? "true" : "false") << "\n";
    file << "}\n";
    
    file.close();
    Log::Info("Saved texture asset: " + filepath);
}

bool TextureImporter::LoadTextureAsset(const std::string& assetPath, TextureSpec& spec) {
    std::ifstream file(assetPath);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple JSON parser (manual, no external dependency)
    // This is a basic implementation - for production, use a proper JSON library
    std::string line;
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Parse key-value pairs
        if (line.find("\"path\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                spec.filepath = line.substr(start + 1, end - start - 1);
            }
        } else if (line.find("\"wrap\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                std::string wrapStr = line.substr(start + 1, end - start - 1);
                if (wrapStr == "ClampToEdge") spec.wrapS = spec.wrapT = TextureWrap::ClampToEdge;
                else if (wrapStr == "ClampToBorder") spec.wrapS = spec.wrapT = TextureWrap::ClampToBorder;
                else if (wrapStr == "MirroredRepeat") spec.wrapS = spec.wrapT = TextureWrap::MirroredRepeat;
                else spec.wrapS = spec.wrapT = TextureWrap::Repeat;
            }
        } else if (line.find("\"filter\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                std::string filterStr = line.substr(start + 1, end - start - 1);
                if (filterStr == "Nearest") {
                    spec.minFilter = TextureFilter::Nearest;
                    spec.magFilter = TextureFilter::Nearest;
                } else {
                    spec.minFilter = TextureFilter::Linear;
                    spec.magFilter = TextureFilter::Linear;
                }
            }
        } else if (line.find("\"gammaCorrected\"") != std::string::npos) {
            spec.gammaCorrected = (line.find("true") != std::string::npos);
        } else if (line.find("\"generateMipmaps\"") != std::string::npos) {
            spec.generateMipmaps = (line.find("true") != std::string::npos);
        }
    }
    
    file.close();
    return true;
}

} // namespace LGE

