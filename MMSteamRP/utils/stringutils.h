#include <globals.h>

inline std::wstring DllFilePath;

inline std::wstring u8string_to_wide_string(char* src)
{
    const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, src, (int)strlen(src), NULL, 0);
    if (size_needed <= 0)
    {
        return L"";
    }

    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, src, (int)strlen(src), &result.at(0), size_needed);
    return result;
}

inline std::string wide_string_to_u8string(wchar_t* src)
{
    const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, src, (int)wcslen(src), NULL, 0, NULL, NULL);
    if (size_needed <= 0)
    {
        return "";
    }
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, src, (int)wcslen(src), &result.at(0), size_needed, NULL, NULL);
    return result;
}

inline std::string to_utf8(std::wstring wstr) {
    return wide_string_to_u8string(&wstr[0]);
}

inline std::wstring to_wstr(std::string utf8) {
    return u8string_to_wide_string(&utf8[0]);
}

inline std::wstring FullPathInDllFolder(std::wstring fileName) {
    std::wstring path = DllFilePath.substr(0, DllFilePath.find_last_of(L"/\\")) + L"\\" + fileName;
    return path;
}

inline std::string FileNameFromPath(std::string path) {
    return path.substr(path.find_last_of("/\\"), path.size());
}