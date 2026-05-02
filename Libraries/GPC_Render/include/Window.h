#pragma once

#ifndef GPC_LINUX
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "ErrorType.h"
#include "Object.h"
#include "glm/vec2.hpp"

class GLFWwindow;

namespace GPC
{

    enum class CursorState : int {
        NORMAL 		= 0x00034001,
        HIDDEN 		= 0x00034002,
        DISABLED	= 0x00034003,
    };

    enum class InputMode : int {
        CURSOR					= 0x00033001,
        STICKY_KEYS				= 0x00033002,
        STICKY_MOUSE_BUTTONS	= 0x00033003,
        LOCK_KEY_MODS			= 0x00033004,
        RAW_MOUSE_MOTION		= 0x00033005,
    };

    GPC_INHERIT_OBJECT(Window)
    {

    public:
        explicit Window();
        ~Window();

        virtual ErrorType Create(const char* title, uint16_t width, uint16_t height, bool fullscreen);
        virtual ErrorType Create();

        [[nodiscard]] ErrorType     GetSurface(VkSurfaceKHR** surface);
        [[nodiscard]] GLFWwindow*   ToGLFWWindow() const { return mp_Window; };

        void Destroy() const;

        virtual void Update();

	    void SetInputMode(InputMode mode, bool state) const;
	    void SetCursorMode(CursorState state) const;
        void Fullscreen(bool windowed);
        void Resize(glm::vec2 size, bool windowed);
        void SetFramerate(float framerate);

        const char* GetTitle() const { return mp_Title; };
        glm::vec2 const& GetSize() const
        {
            return m_Size;
        };

        [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(mp_Window); };

    protected:
        GLFWwindow*     mp_Window           = { nullptr };
        GLFWmonitor*    mp_Monitor          = { nullptr };
        VkSurfaceKHR*   mp_Surface          = { nullptr };

        bool& GetIsResized() { return m_WindowResized; };
        glm::vec2      m_Size              = { 1080, 720 };


    private:
        const char*     mp_Title            = { "" };

        bool            m_FullScreen        = { false };
        bool            m_WindowResized     = { false };

        float           m_Framerate         = { 0.0f };

        // Come from https://github.com/glfw/glfw/issues/1699#issuecomment-3127232010
        static bool GetClosestMonitor(GLFWmonitor** monitor, GLFWwindow* window);

        static void OnKeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
        static void OnMouseCallback(GLFWwindow *window, int button, int action, int mode);
        static void OnScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

    };
} // GPC