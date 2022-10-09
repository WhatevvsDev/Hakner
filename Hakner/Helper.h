#pragma once

#include <string>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace hakner
{
	namespace Log
	{
		enum MessageType
		{
			Default,
			Debug,
			Warning,
			Error,
			RANGE
		};

		namespace
		{
			WORD TypeToColor(MessageType aType)
			{
				switch (aType)
				{
				default:
				case MessageType::Default:
					return 15;
				case MessageType::Debug:
					return 8;
				case MessageType::Warning:
					return 14;
				case MessageType::Error:
					return 12;
				}
			}
		}

		inline void Print(MessageType aType, const char* aFile, int aLineNumber, const std::string& aMessage, bool aAssertion = true)
		{
			// If assertion passed, ignore this message
			if(aAssertion && aType == Error)
				return;

			if(!aAssertion)
				throw std::exception();

			WORD attribute = TypeToColor(aType);

			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(handle, attribute);
			std::string filePath(aFile);

			printf("[%s: %i] - %s\n", filePath.substr(filePath.find_last_of("/\\") + 1).c_str(), aLineNumber, aMessage.c_str());
			SetConsoleTextAttribute(handle, 15);
		}
	}
}

#define LogMsg(type, message) hakner::Log::Print(hakner::type, __FILE__, __LINE__, message, true);
#define LogAssert(message, assertion) hakner::Log::Print(hakner::Log::Error, __FILE__, __LINE__, message, assertion);