#include "common.hpp"

#include <fstream>

namespace foldscape
{
	static std::string g_ProgramFolder;

	void _ThrowIfFailed(VkResult result, const char* lineText, const char* filename, int lineNumber, const char* details)
	{
		if (VK_SUCCESS != result)
		{
			std::stringstream ss;
			ss << "Error in file \"" << filename << " at line " << lineNumber << '\n';
			ss << lineText << '\n';
			ss << "Error code: 0x" << std::hex << result;
			if (details)
				ss << '\n' << details;
			throw std::runtime_error(ss.str().c_str());
		}
	}

	void _ThrowIfFalse(bool statement, const char* lineText, const char* filename, int lineNumber, const char* details)
	{
		if (!statement)
		{
			std::stringstream ss;
			ss << "Error in file \"" << filename << " at line " << lineNumber << '\n';
			ss << lineText;
			if (details)
				ss << '\n' << details;
			throw std::runtime_error(ss.str().c_str());
		}
	}

	void SaveProgramFolder(const char programPath[])
	{
		g_ProgramFolder = GetFolderName(programPath);
	}

	const std::string& GetProgramFolder()
	{
		return g_ProgramFolder;
	}

	std::string GetFolderName(const char filename[])
	{
		uint32_t lastSlash = 0;
		for (uint32_t i = 0; filename[i]; ++i)
			if (filename[i] == '/' || filename[i] == '\\')
				lastSlash = i;
		if (lastSlash)
			return std::string(filename, filename + lastSlash + 1);
		return {};
	}

	std::vector<char> ReadFile(const char* filename)
	{
		std::ifstream infile(filename, std::ios::binary | std::ios::ate);
		if (!infile.is_open())
			return {};
		std::vector<char> buffer(infile.tellg());
		infile.seekg(0);
		infile.read(buffer.data(), buffer.size());
		return buffer;
	}
}
