#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <codecvt>
#include <locale>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <functional>

#ifdef DEBUG
#define VALIDATION_LAYER_ENABLED (true)
#else
#define VALIDATION_LAYER_ENABLED (false)
#endif

namespace foldscape
{
	template <typename T, size_t S>
	constexpr size_t ARRAY_SIZE(T(&)[S]) { return S; }

#define SAFE_DESTROY(func, resource, ...) do {\
		if (VK_NULL_HANDLE != resource)\
		{\
			func(__VA_ARGS__);\
			resource = VK_NULL_HANDLE;\
		}\
	} while(false)

	void _ThrowIfFailed(VkResult result, const char* lineText, const char* filename, int lineNumber, const char* details = nullptr);
	#define ThrowIfFailed(result, ...) ::foldscape::_ThrowIfFailed(result, #result, __FILE__, __LINE__, ##__VA_ARGS__)

	void _ThrowIfFalse(bool statement, const char* lineText, const char* filename, int lineNumber, const char* details = nullptr);
	#define ThrowIfFalse(statement, ...) ::foldscape::_ThrowIfFalse(statement, #statement, __FILE__, __LINE__, ##__VA_ARGS__)

	#define Throw(message) do{\
			std::stringstream ss;\
			ss << "Error in file \"" << __FILE__ << "\" at line " << __LINE__ << '\n';\
			ss << message;\
			throw std::runtime_error(ss.str().c_str());\
		} while(false)

	void SaveProgramFolder(const char programPath[]);
	const std::string& GetProgramFolder();
	std::string GetFolderName(const char filename[]);
	std::vector<char> ReadFile(const char* filename);
}
