#pragma once

#include <Windows.h>

#include <stdexcept>

#include <vector>
#include <algorithm>

namespace RTVR::OpenGL
{
    class BaseFunctionWrapper
    {
        friend class OpenGLFunctionLoader;

    protected:
        virtual VOID Load() = 0;
    };

    class OpenGLFunctionLoader
    {
        static inline std::vector<BaseFunctionWrapper*> OpenGLFunctions;

    public:
        static VOID RegisterFunction(BaseFunctionWrapper* Function);
        static VOID LoadFunctions();
    };

    template<class>
    class OpenGLFunctionWrapper;

    template<class ReturnType, class ... ArgumentType>
    class OpenGLFunctionWrapper<ReturnType(*)(ArgumentType ...)> : private BaseFunctionWrapper
    {
        typedef ReturnType (*OpenGLFunction) (ArgumentType...);
        OpenGLFunction OpenGLFunction_;

        LPCSTR FunctionName_;

        VOID Load() override
        {
            if (!OpenGLFunction_)
            {
                OpenGLFunction_ = reinterpret_cast<OpenGLFunction>(::wglGetProcAddress(FunctionName_));
            }
        }

    public:
        OpenGLFunctionWrapper(LPCSTR FunctionName)
            : OpenGLFunction_ { nullptr }
            , FunctionName_ { FunctionName }
        {
            OpenGLFunctionLoader::RegisterFunction(this);
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