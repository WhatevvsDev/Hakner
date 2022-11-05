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
			// Get Windows Console specific attribute for different text color
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

			// If assertion didn't pass, throw an exception
			if(!aAssertion)
				throw std::exception(aMessage.c_str());

			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

			// Modify color of text
			WORD attribute = TypeToColor(aType);
			SetConsoleTextAttribute(handle, attribute);

			// Get only file name
			std::string fileName { aFile };
			fileName = fileName.substr(fileName.find_last_of("/\\") + 1).c_str();

			// Print and reset color
			printf("[%s: %i] - %s\n", fileName.c_str(), aLineNumber, aMessage.c_str());
			SetConsoleTextAttribute(handle, 15);
		}
	}
}

#define LogMsg(type, message) hakner::Log::Print(hakner::type, __FILE__, __LINE__, message, true);
#define LogAssert(message, assertion) hakner::Log::Print(hakner::Log::Error, __FILE__, __LINE__, message, assertion);