#include "FileHandler.h"

#include <Windows.h>
#include <fstream>
#include <cwchar>
#include <sstream>
#include <stdexcept>

namespace {
std::string ToUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(),
        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) throw std::runtime_error("Could not encode task text.");
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring FromUtf8(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) throw std::runtime_error("The task file contains invalid text.");
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::wstring Escape(const std::wstring& input) {
    std::wstring output;
    for (wchar_t ch : input) {
        if (ch == L'\\') output += L"\\\\";
        else if (ch == L'\t') output += L"\\t";
        else if (ch == L'\n') output += L"\\n";
        else if (ch == L'\r') output += L"\\r";
        else output += ch;
    }
    return output;
}

std::wstring Unescape(const std::wstring& input) {
    std::wstring output;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] != L'\\' || i + 1 >= input.size()) {
            output += input[i];
            continue;
        }
        const wchar_t next = input[++i];
        if (next == L't') output += L'\t';
        else if (next == L'n') output += L'\n';
        else if (next == L'r') output += L'\r';
        else if (next == L'\\') output += L'\\';
        else throw std::runtime_error("The task file contains an invalid escape sequence.");
    }
    return output;
}

std::vector<std::wstring> SplitFields(const std::wstring& line) {
    std::vector<std::wstring> fields;
    size_t start = 0;
    for (size_t i = 0; i < line.size() && fields.size() < 4; ++i) {
        if (line[i] == L'\t') {
            fields.push_back(line.substr(start, i - start));
            start = i + 1;
        }
    }
    fields.push_back(line.substr(start));
    return fields;
}

SYSTEMTIME ParseDate(const std::wstring& value) {
    SYSTEMTIME date{};
    if (swscanf_s(value.c_str(), L"%hu-%hu-%hu", &date.wYear,
                  &date.wMonth, &date.wDay) != 3 || !IsValidDate(date)) {
        throw std::runtime_error("The task file contains an invalid date.");
    }
    return date;
}

SYSTEMTIME ParseTime(const std::wstring& value, const SYSTEMTIME& date) {
    SYSTEMTIME time = date;
    if (swscanf_s(value.c_str(), L"%hu:%hu", &time.wHour, &time.wMinute) != 2 ||
        !IsValidTime(time)) {
        throw std::runtime_error("The task file contains an invalid time.");
    }
    return time;
}
}

void FileHandler::Save(const std::vector<Task>& tasks, const std::filesystem::path& filePath) {
    std::ofstream output(filePath, std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("Could not open the task file for saving.");

    for (const Task& task : tasks) {
        wchar_t date[32]{};
        wchar_t time[16]{};
        swprintf_s(date, L"%04u-%02u-%02u", task.GetDate().wYear,
            task.GetDate().wMonth, task.GetDate().wDay);
        swprintf_s(time, L"%02u:%02u", task.GetTime().wHour, task.GetTime().wMinute);
        std::wstring row = std::to_wstring(task.GetId()) + L"\t" +
            (task.IsCompleted() ? L"1" : L"0") + L"\t" + date + L"\t" + time +
            L"\t" + Escape(task.GetDescription()) + L"\n";
        output << ToUtf8(row);
    }
    if (!output) throw std::runtime_error("A problem occurred while saving tasks.");
}

std::vector<Task> FileHandler::Load(const std::filesystem::path& filePath) {
    if (!std::filesystem::exists(filePath)) return {};
    std::ifstream input(filePath, std::ios::binary);
    if (!input) throw std::runtime_error("Could not open the saved task file.");

    std::vector<Task> loaded;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        const auto fields = SplitFields(FromUtf8(line));
        if (fields.size() != 5) throw std::runtime_error("A saved task row has the wrong format.");

        size_t used = 0;
        const int id = std::stoi(fields[0], &used);
        if (used != fields[0].size() || id < 1)
            throw std::runtime_error("A saved task has an invalid ID.");
        if (fields[1] != L"0" && fields[1] != L"1")
            throw std::runtime_error("A saved task has an invalid completion value.");

        SYSTEMTIME date = ParseDate(fields[2]);
        SYSTEMTIME time = ParseTime(fields[3], date);
        loaded.emplace_back(id, Unescape(fields[4]), date, time, fields[1] == L"1");
    }
    if (input.bad()) throw std::runtime_error("A problem occurred while reading saved tasks.");
    return loaded;
}
