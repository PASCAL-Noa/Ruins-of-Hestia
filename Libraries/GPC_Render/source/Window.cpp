#include "Window.h"

#include "Inputs.h"

#include "ErrorType.h"
#include "RenderDevice.h"

namespace GPC
{
    Window::Window() : m_Size(1080, 720) {}
    Window::~Window() {};

    ErrorType Window::Create(const char *title, uint16_t width, uint16_t height, bool fullscreen)
    {
        mp_Title = title;

        m_Size.x = width;
        m_Size.y = height;

        m_FullScreen = fullscreen;

        return Create();
    }

    ErrorType Window::Create()
    {

        if (!glfwInit())
        {
            return ErrorType::WINDOW_INITIALIZATION_FAILED;
        }
        int platform = glfwGetPlatform();

        switch (platform)
        {
        case GLFW_PLATFORM_X11:
            std::cout << "X11\n";
            break;

        case GLFW_PLATFORM_WAYLAND:
            std::cout << "Wayland\n";
            break;

        case GLFW_PLATFORM_NULL:
            std::cout << "NULL backend\n";
            break;

        default:
            std::cout << "Unknown platform: " << platform << "\n";
            break;
        }
        mp_Monitor = glfwGetPrimaryMonitor();

        glfwWindowHint(GLFW_CLIENT_API,         GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,          GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED,          m_FullScreen);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW,      GLFW_TRUE );
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER , GLFW_TRUE );

        mp_Window = glfwCreateWindow(m_Size.x, m_Size.y, mp_Title, nullptr, nullptr);
        int x, y;
        glfwGetWindowSize(mp_Window, &x, &y);
        m_Size.x = x;
        m_Size.y = y;

        glfwSetKeyCallback(mp_Window, OnKeyboardCallback);
        glfwSetMouseButtonCallback(mp_Window, OnMouseCallback);
        glfwSetScrollCallback(mp_Window, OnScrollCallback);

        glfwSetWindowUserPointer(mp_Window, this);
        glfwSetFramebufferSizeCallback(mp_Window, [](GLFWwindow* window, int width, int height)
        {
            auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            app->m_WindowResized = true;
        });

        return ErrorType::SUCCESS;
    }

    ErrorType Window::GetSurface(VkSurfaceKHR** surface)
    {

        if (mp_Surface != nullptr) {
            *surface = mp_Surface;
            return ErrorType::SUCCESS;
        }

        *surface = new VkSurfaceKHR();
        VkResult err = glfwCreateWindowSurface(RenderDevice::GetVulkanInstance(), mp_Window, nullptr, *surface);
        if (err != VK_SUCCESS) {
            const char* error = nullptr;
            int len = glfwGetError(&error);
            std::cerr << error << std::endl;
            return ErrorType::WINDOW_SURFACE_CREATION_FAILED;
        }
        mp_Surface = *surface;

        return ErrorType::SUCCESS;

    }

    void Window::Destroy() const
    {
        if (mp_Surface != nullptr)
            vkDestroySurfaceKHR(RenderDevice::GetVulkanInstance(), *mp_Surface, nullptr);

        glfwDestroyWindow(mp_Window);

        glfwTerminate();
    }

    void Window::Update()
    {
        glfwPollEvents();
        Inputs::Update(mp_Window);
    }

    void Window::SetInputMode(InputMode mode, bool state) const
    {
        glfwSetInputMode(mp_Window, static_cast<int>(mode), static_cast<int>(state));
    }

    void Window::SetCursorMode(CursorState state) const
    {
        glfwSetInputMode(mp_Window, static_cast<int>(InputMode::CURSOR), static_cast<int>(state));
    }

    void Window::Fullscreen(bool windowed)
    {
        if (windowed)
            glfwMaximizeWindow(mp_Window);
        else {
            GetClosestMonitor(&mp_Monitor, mp_Window);
            const GLFWvidmode* mode = glfwGetVideoMode(mp_Monitor);

            Resize({mode->width, mode->height}, windowed);
        }

    }

    void Window::Resize(glm::vec2 size, bool windowed)
    {

        const GLFWvidmode* mode = glfwGetVideoMode(mp_Monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE );

        m_Size = size;
        int left, top;
        int sizex, sizey;
        glfwGetWindowFrameSize(mp_Window, &left, &top, &sizex, &sizey);
        glfwSetWindowMonitor(mp_Window, windowed ? nullptr : mp_Monitor, mode->width/2 - size.x/2, mode->height/2 - size.y/2, size.x, size.y, m_Framerate);
    }

    void Window::SetFramerate(float framerate)
    {

        m_Framerate = framerate;
        glfwSetWindowMonitor(mp_Window, m_FullScreen ? mp_Monitor : nullptr, 0, 0, m_Size.x, m_Size.y, framerate);

    }

    bool Window::GetClosestMonitor(GLFWmonitor **monitor, GLFWwindow *window)
    {
        bool success = false;

        int window_rectangle[4] = {0};
        glfwGetWindowPos(window, &window_rectangle[0], &window_rectangle[1]);
        glfwGetWindowSize(window, &window_rectangle[2], &window_rectangle[3]);

        int monitors_size = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

        GLFWmonitor* closest_monitor = NULL;
        int max_overlap_area = 0;

        for (int i = 0; i < monitors_size; ++i)
        {
            int monitor_rectangle[4] = {0}; // <-- only position was computed, not rectangle
            glfwGetMonitorWorkarea(monitors[i], &monitor_rectangle[0], &monitor_rectangle[1], &monitor_rectangle[2], &monitor_rectangle[3]);

            const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

            if (
                !(
                    ((window_rectangle[0] + window_rectangle[2]) < monitor_rectangle[0]) ||
                    (window_rectangle[0] > (monitor_rectangle[0] + monitor_rectangle[2])) ||
                    ((window_rectangle[1] + window_rectangle[3]) < monitor_rectangle[1]) ||
                    (window_rectangle[1] > (monitor_rectangle[1] + monitor_rectangle[3]))
                )
            ) {
                int intersection_rectangle[4] = {0};

                // x, width
                if (window_rectangle[0] < monitor_rectangle[0])
                {
                    intersection_rectangle[0] = monitor_rectangle[0];

                    if ((window_rectangle[0] + window_rectangle[2]) < (monitor_rectangle[0] + monitor_rectangle[2]))
                    {
                        intersection_rectangle[2] = (window_rectangle[0] + window_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = monitor_rectangle[2];
                    }
                }
                else
                {
                    intersection_rectangle[0] = window_rectangle[0];

                    if ((monitor_rectangle[0] + monitor_rectangle[2]) < (window_rectangle[0] + window_rectangle[2]))
                    {
                        intersection_rectangle[2] = (monitor_rectangle[0] + monitor_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = window_rectangle[2];
                    }
                }

                // y, height
                if (window_rectangle[1] < monitor_rectangle[1])
                {
                    intersection_rectangle[1] = monitor_rectangle[1];

                    if ((window_rectangle[1] + window_rectangle[3]) < (monitor_rectangle[1] + monitor_rectangle[3]))
                    {
                        intersection_rectangle[3] = (window_rectangle[1] + window_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = monitor_rectangle[3];
                    }
                }
                else
                {
                    intersection_rectangle[1] = window_rectangle[1];

                    if ((monitor_rectangle[1] + monitor_rectangle[3]) < (window_rectangle[1] + window_rectangle[3]))
                    {
                        intersection_rectangle[3] = (monitor_rectangle[1] + monitor_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = window_rectangle[3];
                    }
                }

                int overlap_area = intersection_rectangle[2] * intersection_rectangle[3];  // <-- OOB access here
                if (overlap_area > max_overlap_area)
                {
                    closest_monitor = monitors[i];
                    max_overlap_area = overlap_area;
                }
            }
        }

        if (closest_monitor)
        {
            *monitor = closest_monitor;
            success = true;
        }

        // true: monitor contains the monitor the window is most on
        // false: monitor is unmodified
        return success;
    }

    void Window::OnKeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
    {
        Inputs::UpdateKey(
            static_cast<Inputs::KeyCode>(key),
            (action == GLFW_PRESS || action == GLFW_REPEAT) ? Inputs::KEY_DOWN : Inputs::KEY_UP
        );
    }

    void Window::OnMouseCallback(GLFWwindow *window, int button, int action, int mode)
    {
        Inputs::UpdateMouse(
            static_cast<Inputs::ButtonCode>(button),
            (action == GLFW_PRESS || action == GLFW_REPEAT) ? Inputs::KEY_DOWN : Inputs::KEY_UP
        );
    }

    void Window::OnScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
    {
        Inputs::UpdateScroll(xoffset, yoffset);
    }
} // GPC