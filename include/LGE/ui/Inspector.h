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

#pragma once

#include <memory>

namespace LGE {

class GameObject;
class Texture;
class LightPropertiesComponent;

namespace Luminite {
    class LuminiteSubsystem;
}

class Inspector {
public:
    Inspector();
    ~Inspector();

    void OnUIRender();
    
    void SetSelectedObject(GameObject* obj) { m_SelectedObject = obj; }
    GameObject* GetSelectedObject() const { return m_SelectedObject; }
    
    void SetLuminiteSubsystem(Luminite::LuminiteSubsystem* subsystem) { m_LuminiteSubsystem = subsystem; }

private:
    void LoadIcons();
    void RenderLightProperties(LightPropertiesComponent* lightProps);
    void UpdateLightInLuminite(GameObject* obj, LightPropertiesComponent* lightProps);
    
    GameObject* m_SelectedObject;
    Luminite::LuminiteSubsystem* m_LuminiteSubsystem;
    
    // Icons
    std::shared_ptr<Texture> m_AddIcon;
    bool m_IconsLoaded;
};

} // namespace LGE


