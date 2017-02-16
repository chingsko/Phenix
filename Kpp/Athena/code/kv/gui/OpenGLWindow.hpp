#pragma once

#pragma comment(lib, "opengl32.lib")

#include "Window.hpp"

#include "kv/native/windows/wgl.hpp"
#include "kv/log/IncludeAll.hpp"

using namespace std;
using namespace kv::native::windows;

#include "_namespace/begin"

class OpenGLWindow : public Window
{
protected:
    HDC   DeviceContextHandle_;
    HGLRC RenderingContextHandle_;

public:
    using Window::Window;

    virtual ~OpenGLWindow() noexcept override
    {

    }

protected:

    bool SetupPixelFormat()
    {
        int const format_attributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE                  ,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE                  ,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE                  ,
            WGL_DOUBLE_BUFFER_ARB , GL_TRUE                  ,
            WGL_ACCELERATION_ARB  , WGL_FULL_ACCELERATION_ARB,
            WGL_SWAP_METHOD_ARB   , WGL_SWAP_EXCHANGE_ARB    ,
            WGL_PIXEL_TYPE_ARB    , WGL_TYPE_RGBA_ARB        ,
            WGL_COLOR_BITS_ARB    , 32                       ,
            WGL_DEPTH_BITS_ARB    , 24                       ,
            WGL_STENCIL_BITS_ARB  , 8                        ,
            WGL_SAMPLES_ARB       , 4                        ,
            0
        };

        autox format_index = int{};
        autox format_count = int{};
        if (!wgl::ChoosePixelFormat(DeviceContextHandle_, format_attributes, nullptr, 1, &format_index, &format_count))
        {
            log::error("error: wgl choose pixel format failed.")();
            return false;
        }
        log::info(log::color::green)("info: wgl choose pixel format success.")();

        autox pfd = PIXELFORMATDESCRIPTOR{};
        if (!SetPixelFormat(DeviceContextHandle_, format_index, &pfd))
        {
            log::error("error: set pixel format (index = {0}) failed.", format_index)();
            return false;
        }
        log::info(log::color::green)("info: set pixel format (index = {0}) success.", format_index)();

        return true;
    }

    bool CreateRenderingContext(int const major, int const minor)
    {
        int const context_attributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
#if defined(_DEBUG)
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
#else
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
            0
        };

        RenderingContextHandle_ = wgl::CreateContextAttribs(DeviceContextHandle_, nullptr, context_attributes);
        if (RenderingContextHandle_ == nullptr)
        {
            log::error("error: create rendering context (version {0}.{1}) failed.", major, minor)();
            return false;
        }
        log::info(log::color::green)("info: create rendering context (version {0}.{1}) success.", major, minor)();

        return true;
    }

    virtual bool OnCreate() override
    {
        autox precondition = true
            && Window::OnCreate()
            && wgl::LoadExtensions();

        if (!precondition)
        {
            return false;
        }

        autox window_handle = GetWindowHandle();
        DeviceContextHandle_ = GetDC(window_handle);
        if (DeviceContextHandle_ == nullptr)
        {
            log::error("error: get device handle failed.")();
            return false;
        }

        if (!SetupPixelFormat())
        {
            return false;
        }

        autox versions = vector<pair<int, int>>
        {
            make_pair(4, 5),
            make_pair(4, 4),
            make_pair(4, 3),
            make_pair(4, 2),
            make_pair(4, 1),
            make_pair(4, 0),
            make_pair(3, 3),
            make_pair(3, 2),
            make_pair(3, 1),
            make_pair(3, 0),
            //make_pair(2, 1),
            //make_pair(2, 0),
            //make_pair(1, 5),
            //make_pair(1, 4),
            //make_pair(1, 3),
            //make_pair(1, 2),
            //make_pair(1, 1),
            //make_pair(1, 0),
        };

        autox found = false;
        for (autox v : versions)
        {
            found = CreateRenderingContext(v.first, v.second);
            if (found) break;
        }
        if (!found)
        {
            return false;
        }

        wgl::MakeCurrent(DeviceContextHandle_, RenderingContextHandle_);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glClearDepth(1.0f);
        glClearColor(1, 0, 1, 1);

        return true;
    }

    virtual LRESULT CALLBACK OnEvent(UINT message, WPARAM wparam, LPARAM lparam) override
    {
        return Window::OnEvent(message, wparam, lparam);
    }

    virtual void DoRender() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SwapBuffers(GetDC(GetWindowHandle()));
    }
};

#include "_namespace/end"