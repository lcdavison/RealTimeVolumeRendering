#pragma once

#include <Windows.h>
#include <functional>
#include <stdexcept>

namespace RTVR::ErrorHandling
{
	template<class FunctionType, class ... Arguments>
	VOID ExecuteAndCatchException(FunctionType FunctionToExecute, Arguments ... FunctionArguments)
	{
		static_assert(std::is_invocable<FunctionType, Arguments...>::value, "Parameter is not a function");

		constexpr UINT ErrorMessageMaxLength { 512 };

		try
		{
			FunctionToExecute(FunctionArguments...);
		}
		catch (std::runtime_error RuntimeError)
		{
			WCHAR ErrorMessage [ErrorMessageMaxLength];
			::mbstowcs_s(nullptr, ErrorMessage, RuntimeError.what(), ErrorMessageMaxLength);

			::MessageBox(nullptr, ErrorMessage, L"Exception Thrown", MB_OK);
		}
		catch (...)
		{
			::MessageBox(nullptr, L"An unknown exception has been thrown", L"Exception Thrown", MB_OK);
		}
	}
}