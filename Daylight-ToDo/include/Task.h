#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <string>

class Task {
public:
    Task() = default;
    Task(int id, std::wstring description, const SYSTEMTIME& date,
         const SYSTEMTIME& time, bool completed = false);

    int GetId() const;
    const std::wstring& GetDescription() const;
    const SYSTEMTIME& GetDate() const;
    const SYSTEMTIME& GetTime() const;
    bool IsCompleted() const;

    void SetDescription(const std::wstring& description);
    void SetDate(const SYSTEMTIME& date);
    void SetTime(const SYSTEMTIME& time);
    void SetCompleted(bool completed);

private:
    int m_id{ 0 };
    std::wstring m_description;
    SYSTEMTIME m_date{};
    SYSTEMTIME m_time{};
    bool m_completed{ false };
};

bool IsDateBeforeToday(const SYSTEMTIME& date);
bool IsValidDate(const SYSTEMTIME& date);
bool IsValidTime(const SYSTEMTIME& time);
std::wstring FormatDate(const SYSTEMTIME& date);
std::wstring FormatTime(const SYSTEMTIME& time);
