#include "Inputs.h"

namespace GPC
{
    void Inputs::UpdateKey(KeyCode code, InputStatus status) {
        InputStatus next = SLEEPING;
        InputStatus previous = m_statusKey[KeyCodeToIndex(code)];

        if (status & KEY_DOWN) next |= KEY_DOWN;
        else next |= KEY_UP;

        const bool WAS_UP = previous & KEY_UP;
        const bool WAS_DOWN = previous & KEY_DOWN;
        const bool IS_UP = status & KEY_UP;
        const bool IS_DOWN = status & KEY_DOWN;
        const bool WAS_PRESS_LOCKED = previous & KEY_NO_PRESS;
        const bool WAS_RELEASE_LOCKED = previous & KEY_NO_RELEASE;

        if (WAS_PRESS_LOCKED && !IS_UP) next |= KEY_NO_PRESS;
        if (WAS_RELEASE_LOCKED && !IS_DOWN) next |= KEY_NO_RELEASE;

        if (WAS_UP && IS_DOWN && !WAS_PRESS_LOCKED) {
            next |= KEY_PRESS;

        }
        else if (WAS_DOWN && IS_UP && !WAS_RELEASE_LOCKED) {
            next |= KEY_RELEASE;
        }

        m_statusKey[KeyCodeToIndex(code)] = next;
    }

    void Inputs::UpdateMouse(ButtonCode code, InputStatus status) {
        InputStatus next = SLEEPING;
        InputStatus previous = m_statusMouse[ButtonCodeToIndex(code)];

        if (status & KEY_DOWN) next |= KEY_DOWN;
        else next |= KEY_UP;

        const bool WAS_UP = previous & KEY_UP;
        const bool WAS_DOWN = previous & KEY_DOWN;
        const bool IS_UP = status & KEY_UP;
        const bool IS_DOWN = status & KEY_DOWN;
        const bool WAS_PRESS_LOCKED = previous & KEY_NO_PRESS;
        const bool WAS_RELEASE_LOCKED = previous & KEY_NO_RELEASE;

        if (WAS_PRESS_LOCKED && !IS_UP) next |= KEY_NO_PRESS;
        if (WAS_RELEASE_LOCKED && !IS_DOWN) next |= KEY_NO_RELEASE;

        if (!WAS_DOWN && IS_DOWN) {
            next |= KEY_JUST_PRESS;
        }

        if (WAS_UP && IS_DOWN && !WAS_PRESS_LOCKED) {
            next |= KEY_PRESS;
        }
        else if (WAS_DOWN && IS_UP && !WAS_RELEASE_LOCKED) {
            next |= KEY_RELEASE;
        }

         m_statusMouse[ButtonCodeToIndex(code)] = next;
    }

    void Inputs::UpdateScroll(float xoffset, float yoffset) {
        m_deltaMouseScrollX = xoffset;
        m_deltaMouseScrollY = yoffset;
    }

    void Inputs::Update(GLFWwindow* _win) {

        float previous_mouse_pos_x = m_mousePosX;
        float previous_mouse_pos_y = m_mousePosY;

        double current_mouse_pos_x;
        double current_mouse_pos_y;
        glfwGetCursorPos(_win, &current_mouse_pos_x, &current_mouse_pos_y);

        m_mousePosX = static_cast<float>(current_mouse_pos_x);
        m_mousePosY = static_cast<float>(current_mouse_pos_y);

        if (m_mousePosX == 0 && m_mousePosY == 0) {
            m_deltaMousePosX = 0;
            m_deltaMousePosY = 0;
        } else {
            m_deltaMousePosX = m_mousePosX - previous_mouse_pos_x;
            m_deltaMousePosY = m_mousePosY - previous_mouse_pos_y;
        }
    }

    void Inputs::UpdateStates()
    {

        for (uint8_t& status : m_statusMouse) {
            if (status & KEY_JUST_PRESS) {
                status ^= KEY_JUST_PRESS;
            }
        }

    }

    ErrorType Inputs::Create() {
        for (int i = 0; i < KEY_COUNT; ++i) {
            m_statusKey[i] = KEY_UP;
        }
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            m_statusKey[i] = KEY_UP;
        }
        return ErrorType::SUCCESS;
    }

    void Inputs::Destroy() {

    }

    bool Inputs::IsKeyDown(KeyCode code) {
        return m_statusKey[KeyCodeToIndex(code)] & KEY_DOWN;
    }

    bool Inputs::IsKeyUp(KeyCode code) {
        return m_statusKey[KeyCodeToIndex(code)] & KEY_UP;
    }

    bool Inputs::IsKeyPress(KeyCode code) {
        const bool IS_PRESS = m_statusKey[KeyCodeToIndex(code)] & KEY_PRESS;
        const bool IS_PRESS_LOCKED = m_statusKey[KeyCodeToIndex(code)] & KEY_NO_PRESS;
        if (IS_PRESS) {
            m_statusKey[KeyCodeToIndex(code)] |= KEY_NO_PRESS;
        }
        return IS_PRESS && !IS_PRESS_LOCKED;
    }

    bool Inputs::IsKeyRelease(KeyCode code) {
        const bool IS_RELEASE = m_statusKey[KeyCodeToIndex(code)] & KEY_RELEASE;
        const bool IS_RELEASE_LOCKED = m_statusKey[KeyCodeToIndex(code)] & KEY_NO_RELEASE;
        if (IS_RELEASE) {
            m_statusKey[KeyCodeToIndex(code)] |= KEY_NO_RELEASE;
        }
        return IS_RELEASE && !IS_RELEASE_LOCKED;
    }

    bool Inputs::IsLetterTyped(char &out) {
        for (uint16_t i = static_cast<uint16_t>(KeyCode::A); i <= static_cast<uint16_t>(KeyCode::Z); ++i) {
            if (IsKeyPress(static_cast<KeyCode>(i))) {
                out = static_cast<char>(i);
                return true;
            }
        }
        return false;
    }

    bool Inputs::IsButtonDown(ButtonCode code) {
        return m_statusMouse[ButtonCodeToIndex(code)] & KEY_DOWN;
    }

    bool Inputs::IsButtonUp(ButtonCode code) {
        return m_statusMouse[ButtonCodeToIndex(code)] & KEY_UP;
    }

    bool Inputs::IsButtonPress(ButtonCode code) {
        return m_statusMouse[ButtonCodeToIndex(code)] & KEY_JUST_PRESS;
    }

    bool Inputs::IsButtonRelease(ButtonCode code) {
        const bool IS_RELEASE = m_statusMouse[ButtonCodeToIndex(code)] & KEY_RELEASE;
        const bool IS_RELEASE_LOCKED = m_statusMouse[ButtonCodeToIndex(code)] & KEY_NO_RELEASE;
        if (IS_RELEASE) {
            m_statusMouse[ButtonCodeToIndex(code)] |= KEY_NO_RELEASE;
        }
        return IS_RELEASE && !IS_RELEASE_LOCKED;
    }

    float Inputs::GetScrollOffsetX() {
        auto r = m_deltaMouseScrollX;
        m_deltaMouseScrollX = 0.0f;
        return r;
    }

    float Inputs::GetScrollOffsetY() {
        auto r = m_deltaMouseScrollY;
        m_deltaMouseScrollY = 0.0f;
        return r;
    }

    float Inputs::GetMouseX() {
        return m_mousePosX;
    }

    float Inputs::GetMouseY() {
        return m_mousePosY;
    }

    float Inputs::GetDeltaMouseX() {
        return m_deltaMousePosX;
    }

    float Inputs::GetDeltaMouseY() {
        return m_deltaMousePosY;
    }

} // GPC