#include "OpenGLFunction.h"

namespace RTVR::OpenGL
{
	VOID OpenGLFunctionLoader::RegisterFunction(BaseFunctionWrapper* Function)
	{
		OpenGLFunctions.push_back(Function);
	}

	VOID OpenGLFunctionLoader::LoadFunctions()
	{
		auto LoadFunction = [](BaseFunctionWrapper* Function)
		{
			Function->Load();
		};

		std::for_each(OpenGLFunctions.begin(), OpenGLFunctions.end(), LoadFunction);

		OpenGLFunctions.clear();
	}
}