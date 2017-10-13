/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#ifndef UNICORN_VIDEO_CAMERAFPSCONTROLLER_HPP
#define UNICORN_VIDEO_CAMERAFPSCONTROLLER_HPP

#include <unicorn/utility/SharedMacros.hpp>
#include <unicorn/video/CameraController.hpp>

namespace unicorn
{
namespace video
{

/** @brief FPS style camera controller */
class CameraFpsController : public CameraController
{
public:
    UNICORN_EXPORT CameraFpsController(glm::mat4& cameraView);

    /** @brief Updates view matrix by taking diff between past and new mouse coordinates */
    UNICORN_EXPORT void UpdateView(float x, float y);

    /** @brief Sets mouse coordinates without updating view matrix */
    UNICORN_EXPORT void SetViewPositions(double x, double y);

    UNICORN_EXPORT void Roll(float angleRadians)
    {
        m_roll += angleRadians;
    }

    //! Mouse sensitivity for x axis
    float sensitivityX;
    //! Mouse sensitivity for y axis
    float sensitivityY;
    //! Mouse sensitivity for z axis
    float sensitivityZ;
protected:
    virtual void UpdateViewMatrix();
private:
    glm::vec2 m_mousePosition;
    float m_yaw;
    float m_pitch;
    float m_roll;
    bool m_dirtyViewPosition;
};

} // namespace video
} // namespace unicorn

#endif // UNICORN_VIDEO_CAMERAFPSCONTROLLER_HPP
