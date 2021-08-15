#pragma once

#include <stdexcept>

namespace RTVR::OpenGL
{
	template<class>
	class OpenGLFunctionWrapper;

	template<class ReturnType, class ... ArgumentType>
	class OpenGLFunctionWrapper<ReturnType(*)(ArgumentType ...)>
	{
		typedef ReturnType (*OpenGLFunction) (ArgumentType...);
		OpenGLFunction OpenGLFunction_;

		LPCSTR FunctionName_;

	public:
		OpenGLFunctionWrapper(LPCSTR FunctionName)
			: OpenGLFunction_ { nullptr }
			, FunctionName_ { FunctionName }
		{
		}

		VOID Load()
		{
			if (!OpenGLFunction_)
			{
				OpenGLFunction_ = reinterpret_cast<OpenGLFunction>(::wglGetProcAddress(FunctionName_));
			}
		}

		ReturnType operator()(ArgumentType ... Arguments)
		{
			if (!OpenGLFunction_)
			{
				throw std::runtime_error("Function has not been loaded");
			}

			return OpenGLFunction_(Arguments ...);
		}
	};
}