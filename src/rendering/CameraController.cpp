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

#include "LGE/rendering/CameraController.h"
#include "LGE/core/Input.h"
#include "LGE/ui/UI.h"
#include "LGE/math/Vector.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

namespace LGE {

CameraController::CameraController(Camera* camera)
    : m_Camera(camera)
    , m_MovementSpeed(5.0f)
    , m_PanSpeed(2.0f)
    , m_ZoomSpeed(2.0f)
    , m_MouseSensitivity(0.1f)
    , m_MinZoomDistance(0.5f)
    , m_MaxZoomDistance(20.0f)
    , m_LastMouseX(0.0)
    , m_LastMouseY(0.0)
    , m_FirstMouse(true)
    , m_Yaw(-90.0f)
    , m_Pitch(0.0f)
{
    if (m_Camera) {
        Math::Vector3 position = m_Camera->GetPosition();
        Math::Vector3 target = m_Camera->GetTarget();
        Math::Vector3 direction = Math::Vector3(
            target.x - position.x,
            target.y - position.y,
            target.z - position.z
        );
        
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (length > 0.0f) {
            direction = Math::Vector3(direction.x / length, direction.y / length, direction.z / length);
        }
        
        // Calculate initial yaw and pitch from direction
        float lengthXZ = std::sqrt(direction.x * direction.x + direction.z * direction.z);
        m_Yaw = std::atan2(direction.x, direction.z) * 180.0f / 3.14159f;
        m_Pitch = std::atan2(direction.y, lengthXZ) * 180.0f / 3.14159f;
    }
}

void CameraController::OnUpdate(float deltaTime) {
    ProcessKeyboard(deltaTime);
    ProcessMouse(deltaTime);
    ProcessScroll(deltaTime);
}

void CameraController::OnWindowResize(uint32_t width, uint32_t height) {
    if (m_Camera) {
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_Camera->SetPerspective(45.0f, aspectRatio, 0.1f, 100.0f);
    }
}

void CameraController::ProcessKeyboard(float deltaTime) {
    if (!m_Camera) return;

    Math::Vector3 position = m_Camera->GetPosition();
    Math::Vector3 target = m_Camera->GetTarget();
    Math::Vector3 direction = Math::Vector3(
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    );
    
    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0f) {
        direction = Math::Vector3(direction.x / length, direction.y / length, direction.z / length);
    }
    
    // Calculate right vector (cross product of direction and up)
    Math::Vector3 up = m_Camera->GetUp();
    Math::Vector3 right = Math::Vector3(
        direction.y * up.z - direction.z * up.y,
        direction.z * up.x - direction.x * up.z,
        direction.x * up.y - direction.y * up.x
    );
    
    // Normalize right vector
    length = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    if (length > 0.0f) {
        right = Math::Vector3(right.x / length, right.y / length, right.z / length);
    }
    
    // Calculate forward vector (only XZ plane for movement)
    Math::Vector3 forward = Math::Vector3(direction.x, 0.0f, direction.z);
    length = std::sqrt(forward.x * forward.x + forward.z * forward.z);
    if (length > 0.0f) {
        forward = Math::Vector3(forward.x / length, 0.0f, forward.z / length);
    }
    
    Math::Vector3 movement(0.0f, 0.0f, 0.0f);
    float speed = m_MovementSpeed * deltaTime;
    
    // WASD movement
    if (Input::IsKeyPressed(GLFW_KEY_W)) {
        movement = movement + forward * speed;
    }
    if (Input::IsKeyPressed(GLFW_KEY_S)) {
        movement = movement + forward * (-speed);
    }
    if (Input::IsKeyPressed(GLFW_KEY_A)) {
        movement = movement + right * (-speed);
    }
    if (Input::IsKeyPressed(GLFW_KEY_D)) {
        movement = movement + right * speed;
    }
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) {
        movement = movement + up * speed;
    }
    if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        movement = movement + up * (-speed);
    }
    
    if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f) {
        m_Camera->Move(movement);
    }
}

void CameraController::ProcessMouse(float deltaTime) {
    if (!m_Camera) return;
    
    // Only block camera input if UI wants to capture mouse AND we're not interacting with viewport
    // This allows camera controls when viewport is focused
    // UI::WantCaptureMouse() returns true when hovering over UI elements, but we want camera to work
    // when hovering over the viewport image itself
    // We'll be more permissive - only block if we're clearly interacting with UI widgets
    
    double mouseX, mouseY;
    Input::GetMousePosition(mouseX, mouseY);
    
    // Process right mouse button rotation
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (m_FirstMouse) {
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
            m_FirstMouse = false;
        }
        
        float xOffset = static_cast<float>(mouseX - m_LastMouseX) * m_MouseSensitivity;
        float yOffset = static_cast<float>(m_LastMouseY - mouseY) * m_MouseSensitivity; // Reversed since y-coordinates go from bottom to top
        
        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;
        
        m_Yaw += xOffset;
        m_Pitch += yOffset;
        
        // Constrain pitch
        if (m_Pitch > 89.0f) m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
        
        // Calculate new direction
        float yawRad = m_Yaw * 3.14159f / 180.0f;
        float pitchRad = m_Pitch * 3.14159f / 180.0f;
        
        Math::Vector3 direction(
            std::cos(pitchRad) * std::cos(yawRad),
            std::sin(pitchRad),
            std::cos(pitchRad) * std::sin(yawRad)
        );
        
        // Normalize direction
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (length > 0.0f) {
            direction = Math::Vector3(direction.x / length, direction.y / length, direction.z / length);
        }
        
        Math::Vector3 position = m_Camera->GetPosition();
        Math::Vector3 currentTarget = m_Camera->GetTarget();
        
        // Calculate current distance from camera to target
        Math::Vector3 currentDirection = Math::Vector3(
            currentTarget.x - position.x,
            currentTarget.y - position.y,
            currentTarget.z - position.z
        );
        float distance = std::sqrt(currentDirection.x * currentDirection.x + 
                                   currentDirection.y * currentDirection.y + 
                                   currentDirection.z * currentDirection.z);
        
        // If distance is 0 or invalid, use default distance
        if (distance < 0.1f) {
            distance = 3.0f;
        }
        
        // Set new target maintaining the same distance
        Math::Vector3 target = Math::Vector3(
            position.x + direction.x * distance,
            position.y + direction.y * distance,
            position.z + direction.z * distance
        );
        
        m_Camera->SetTarget(target);
    } else {
        m_FirstMouse = true;
    }
    
    // Process middle mouse button panning (independent of rotation)
    static double lastPanX = 0.0;
    static double lastPanY = 0.0;
    static bool firstPan = true;
    
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        if (firstPan) {
            lastPanX = mouseX;
            lastPanY = mouseY;
            firstPan = false;
        }
        
        // Calculate mouse delta (pixel movement) - inverted
        float panDeltaX = static_cast<float>(lastPanX - mouseX); // Inverted
        float panDeltaY = static_cast<float>(mouseY - lastPanY); // Inverted
        
        // Apply pan sensitivity
        float panSensitivity = m_PanSpeed * 0.005f; // Increased sensitivity for better responsiveness
        panDeltaX *= panSensitivity;
        panDeltaY *= panSensitivity;
        
        Math::Vector3 position = m_Camera->GetPosition();
        Math::Vector3 target = m_Camera->GetTarget();
        
        // Calculate camera direction (from position to target)
        Math::Vector3 camDirection = Math::Vector3(
            target.x - position.x,
            target.y - position.y,
            target.z - position.z
        );
        float length = std::sqrt(camDirection.x * camDirection.x + camDirection.y * camDirection.y + camDirection.z * camDirection.z);
        if (length > 0.0f) {
            camDirection = Math::Vector3(camDirection.x / length, camDirection.y / length, camDirection.z / length);
        }
        
        // Calculate right vector (perpendicular to view direction and up)
        Math::Vector3 camUp = m_Camera->GetUp();
        Math::Vector3 camRight = Math::Vector3(
            camDirection.y * camUp.z - camDirection.z * camUp.y,
            camDirection.z * camUp.x - camDirection.x * camUp.z,
            camDirection.x * camUp.y - camDirection.y * camUp.x
        );
        
        // Normalize right vector
        length = std::sqrt(camRight.x * camRight.x + camRight.y * camRight.y + camRight.z * camRight.z);
        if (length > 0.0f) {
            camRight = Math::Vector3(camRight.x / length, camRight.y / length, camRight.z / length);
        }
        
        // Calculate proper up vector (perpendicular to both direction and right)
        Math::Vector3 properUp = Math::Vector3(
            camRight.y * camDirection.z - camRight.z * camDirection.y,
            camRight.z * camDirection.x - camRight.x * camDirection.z,
            camRight.x * camDirection.y - camRight.y * camDirection.x
        );
        
        // Normalize up vector
        length = std::sqrt(properUp.x * properUp.x + properUp.y * properUp.y + properUp.z * properUp.z);
        if (length > 0.0f) {
            properUp = Math::Vector3(properUp.x / length, properUp.y / length, properUp.z / length);
        }
        
        // Pan in the plane perpendicular to view direction
        // Right = left/right panning, Up = up/down panning
        Math::Vector3 panOffset = camRight * panDeltaX + properUp * panDeltaY;
        m_Camera->Pan(panOffset);
        
        lastPanX = mouseX;
        lastPanY = mouseY;
    } else {
        firstPan = true;
    }
}

void CameraController::ProcessScroll(float deltaTime) {
    if (!m_Camera) return;
    
    double scrollX, scrollY;
    Input::GetScrollOffset(scrollX, scrollY);
    
    if (scrollY != 0.0) {
        Math::Vector3 position = m_Camera->GetPosition();
        Math::Vector3 target = m_Camera->GetTarget();
        
        // Calculate current distance from camera to target
        Math::Vector3 direction = Math::Vector3(
            target.x - position.x,
            target.y - position.y,
            target.z - position.z
        );
        float currentDistance = std::sqrt(direction.x * direction.x + 
                                          direction.y * direction.y + 
                                          direction.z * direction.z);
        
        // Calculate distance-based zoom speed
        // Zoom faster when far away, slower when close
        // Use logarithmic scaling for smooth transitions
        float normalizedDistance = (currentDistance - m_MinZoomDistance) / (m_MaxZoomDistance - m_MinZoomDistance);
        normalizedDistance = std::max(0.0f, std::min(1.0f, normalizedDistance)); // Clamp to 0-1
        
        // Speed multiplier: faster when far (high distance), slower when close (low distance)
        // Range from 0.5x (close) to 8.0x (far) for faster zooming
        float distanceMultiplier = 0.5f + normalizedDistance * 7.5f; // 0.5 to 8.0
        
        // Calculate zoom delta with distance-based speed
        float baseZoomDelta = static_cast<float>(scrollY) * m_ZoomSpeed * deltaTime;
        float zoomDelta = baseZoomDelta * distanceMultiplier;
        
        // Additional adjustment near limits for fine control
        float limitMultiplier = 1.0f;
        if (currentDistance >= m_MaxZoomDistance * 0.9f) {
            // Near max zoom out - increase sensitivity slightly
            float factor = (currentDistance - m_MaxZoomDistance * 0.9f) / (m_MaxZoomDistance * 0.1f);
            limitMultiplier = 1.0f + factor * 0.3f; // Up to 30% increase
        } else if (currentDistance <= m_MinZoomDistance * 1.1f) {
            // Near min zoom in - decrease sensitivity slightly
            float factor = 1.0f - (currentDistance - m_MinZoomDistance) / (m_MinZoomDistance * 0.1f);
            limitMultiplier = 1.0f - factor * 0.3f; // Up to 30% decrease
        }
        
        zoomDelta *= limitMultiplier;
        
        // Calculate new distance
        float newDistance = currentDistance - zoomDelta; // Negative zoomDelta zooms in (decreases distance)
        
        // Clamp to limits
        if (newDistance < m_MinZoomDistance) {
            newDistance = m_MinZoomDistance;
        } else if (newDistance > m_MaxZoomDistance) {
            newDistance = m_MaxZoomDistance;
        }
        
        // Only zoom if we're not at the limit or moving away from it
        if ((scrollY > 0.0 && currentDistance > m_MinZoomDistance) || 
            (scrollY < 0.0 && currentDistance < m_MaxZoomDistance)) {
            // Normalize direction
            if (currentDistance > 0.0f) {
                direction = Math::Vector3(direction.x / currentDistance, 
                                         direction.y / currentDistance, 
                                         direction.z / currentDistance);
            }
            
            // Set new position maintaining direction
            Math::Vector3 newPosition = Math::Vector3(
                target.x - direction.x * newDistance,
                target.y - direction.y * newDistance,
                target.z - direction.z * newDistance
            );
            
            m_Camera->SetPosition(newPosition);
        }
    }
}

} // namespace LGE
