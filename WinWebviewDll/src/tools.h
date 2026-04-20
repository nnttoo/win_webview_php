#pragma once
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>

#include <objbase.h> // Wajib untuk CoTaskMemAlloc 
#include <shellapi.h>     // WAJIB untuk CommandLineToArgvW
#include <objbase.h>      // WAJIB untuk CoTaskMemAlloc
 
#include <filesystem>
std::string DecodeURIComponent(const std::string& encodedStr) {
	std::string decodedStr;

	size_t length = encodedStr.length();
	size_t index = 0;

	while (index < length)
	{
		if (encodedStr[index] == '%')
		{
			if (index + 2 < length)
			{
				// Decode hexadecimal representation
				std::string hexStr = encodedStr.substr(index + 1, 2);
				int hexValue = std::stoi(hexStr, nullptr, 16);
				decodedStr.push_back(static_cast<char>(hexValue));
				index += 3;
			}
			else
			{
				// Invalid format, append '%' as is
				decodedStr.push_back(encodedStr[index]);
				index++;
			}
		}
		else
		{
			// Append character as is
			decodedStr.push_back(encodedStr[index]);
			index++;
		}
	}

	return decodedStr;
}

std::wstring ConvertToWideString(const std::string& str)
{
	int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring wideStr(length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wideStr[0], length);
	return wideStr;
}

std::string ConvertFromWideString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();

    // 1. Hitung panjang buffer yang dibutuhkan (dalam bytes)
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    
    // 2. Siapkan string tujuan dengan ukuran tersebut
    std::string strTo(size_needed, 0);
    
    // 3. Lakukan konversi aktual
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    
    return strTo;
}

LPCWSTR ConvertToLPCWSTR(const std::string& str) {
	// Konversi string menjadi wide string UTF-16
	int wideStrLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::vector<wchar_t> wideStr(wideStrLength);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideStr.data(), wideStrLength);

	// Mengalokasikan buffer dan menyalin wide string ke dalamnya
	size_t bufferSize = (wideStrLength + 1) * sizeof(wchar_t);  // Ukuran buffer dalam byte
	LPWSTR buffer = static_cast<LPWSTR>(CoTaskMemAlloc(bufferSize));
	if (buffer != nullptr) {
		memcpy(buffer, wideStr.data(), bufferSize);
		buffer[wideStrLength] = L'\0';  // Menambahkan karakter null di akhir wide string
	}

	return buffer;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);

	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}

	tokens.push_back(str.substr(start));

	return tokens;
}
std::vector<std::wstring> SplitStringW(const std::wstring& str, char delimiter) {
	std::vector<std::wstring> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);

	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}

	tokens.push_back(str.substr(start));

	return tokens;
}

std::vector<std::string> ParseArguments(LPSTR lpCmdLine)
{
	std::vector<std::string> arguments;

	std::string cmdLine(lpCmdLine);
	std::string argument;

	bool insideQuotes = false;

	for (char c : cmdLine)
	{
		if (c == '\"')
		{
			insideQuotes = !insideQuotes;
		}
		else if (c == ' ' && !insideQuotes)
		{
			if (!argument.empty())
			{
				arguments.push_back(argument);
				argument.clear();
			}
		}
		else
		{
			argument += c;
		}
	}

	if (!argument.empty())
	{
		arguments.push_back(argument);
	}

	return arguments;
}


namespace fs = std::filesystem;

std::wstring JoinToDllPath(const std::wstring& filename) {
	wchar_t buffer[MAX_PATH];
	HMODULE hMod = NULL;

	// 1. Dapatkan handle DLL ini sendiri berdasarkan alamat fungsi ini
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(&JoinToDllPath),
		&hMod))
	{
		// 2. Ambil path lengkap file .dll (misal: C:\App\bin\MyLib.dll)
		GetModuleFileNameW(hMod, buffer, MAX_PATH);

		// 3. Gunakan filesystem untuk ambil folder dan gabungkan
		fs::path dllPath(buffer);
		fs::path finalPath = dllPath.parent_path() / filename;

		return finalPath.wstring();
	}

	return filename; // Fallback jika gagal, kembalikan filename aslinya
}