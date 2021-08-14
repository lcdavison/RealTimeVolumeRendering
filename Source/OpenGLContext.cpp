#include "OpenGLContext.h"

#include <stdexcept>

#include <gl/GL.h>

namespace RTVR::OpenGL
{
	class Context::Implementation
	{
		HDC DeviceContext_;
		HGLRC OpenGLContext_;

	public:
		VOID CreateContext(HWND WindowHandle);
		VOID DeleteContext();

		VOID SetupPixelFormat();

		VOID SwapBuffers();

	};

	VOID Context::Implementation::CreateContext(HWND WindowHandle)
	{
		DeviceContext_ = { ::GetDC(WindowHandle) };

		SetupPixelFormat();

		OpenGLContext_ = ::wglCreateContext(DeviceContext_);
		::wglMakeCurrent(DeviceContext_, OpenGLContext_);
	}

	VOID Context::Implementation::SetupPixelFormat()
	{
		PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};

		PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		PixelFormatDescriptor.nVersion = 1;
		PixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		PixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
		PixelFormatDescriptor.cColorBits = 32;
		PixelFormatDescriptor.cDepthBits = 24;
		PixelFormatDescriptor.cStencilBits = 8;
		PixelFormatDescriptor.cAccumBits = 0;

		INT PixelFormatIndex = { ::ChoosePixelFormat(DeviceContext_, &PixelFormatDescriptor) };

		if (PixelFormatIndex == 0)
		{
			throw std::runtime_error("Failed to choose pixel format");
		}

		BOOL IsSetPixelFormatSuccessful = { ::SetPixelFormat(DeviceContext_, PixelFormatIndex, &PixelFormatDescriptor) };

		if (!IsSetPixelFormatSuccessful)
		{
			throw std::runtime_error("Failed to set pixel format");
		}
	}

	VOID Context::Implementation::DeleteContext()
	{
		::wglDeleteContext(OpenGLContext_);
	}

	VOID Context::Implementation::SwapBuffers()
	{
		::SwapBuffers(DeviceContext_);
	}
}

namespace RTVR::OpenGL
{
	Context::Context(HWND WindowHandle)
		: Impl_ { new Context::Implementation{} }
	{
		Impl_->CreateContext(WindowHandle);
	}

	Context::~Context()
	{
		Impl_->DeleteContext();
	}

	VOID Context::ClearBackBuffer()
	{
		glClearColor(0.0f, 0.5f, 0.5f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	VOID Context::SwapBuffers()
	{
		Impl_->SwapBuffers();
	}
}