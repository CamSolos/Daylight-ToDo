#include "Task.h"

#include <stdexcept>
#include <utility>

Task::Task(int id, std::wstring description, const SYSTEMTIME& date,
           const SYSTEMTIME& time, bool completed)
    : m_id(id), m_description(std::move(description)),
      m_date(date), m_time(time), m_completed(completed) {
    SetDescription(m_description);
    SetDate(m_date);
    SetTime(m_time);
}

int Task::GetId() const { return m_id; }
const std::wstring& Task::GetDescription() const { return m_description; }
const SYSTEMTIME& Task::GetDate() const { return m_date; }
const SYSTEMTIME& Task::GetTime() const { return m_time; }
bool Task::IsCompleted() const { return m_completed; }

void Task::SetDescription(const std::wstring& description) {
    if (description.find_first_not_of(L" \t\r\n") == std::wstring::npos) {
        throw std::invalid_argument("Please enter a task description.");
    }
    m_description = description;
}

void Task::SetDate(const SYSTEMTIME& date) {
    if (!IsValidDate(date)) {
        throw std::invalid_argument("Please choose a valid date.");
    }
    m_date = date;
}

void Task::SetTime(const SYSTEMTIME& time) {
    if (!IsValidTime(time)) {
        throw std::invalid_argument("Please choose a valid time.");
    }
    m_time = time;
}

void Task::SetCompleted(bool completed) { m_completed = completed; }

bool IsValidDate(const SYSTEMTIME& date) {
    SYSTEMTIME value = date;
    FILETIME fileTime{};
    return SystemTimeToFileTime(&value, &fileTime) != FALSE;
}

bool IsDateBeforeToday(const SYSTEMTIME& date) {
    SYSTEMTIME current{};
    GetLocalTime(&current);
    if (date.wYear != current.wYear) return date.wYear < current.wYear;
    if (date.wMonth != current.wMonth) return date.wMonth < current.wMonth;
    return date.wDay < current.wDay;
}

bool IsValidTime(const SYSTEMTIME& time) {
    return time.wHour <= 23 && time.wMinute <= 59;
}

std::wstring FormatDate(const SYSTEMTIME& date) {
    wchar_t buffer[40]{};
    swprintf_s(buffer, L"%02u/%02u/%04u", date.wMonth, date.wDay, date.wYear);
    return buffer;
}

std::wstring FormatTime(const SYSTEMTIME& time) {
    wchar_t buffer[40]{};
    const unsigned hour = (time.wHour % 12 == 0) ? 12 : time.wHour % 12;
    swprintf_s(buffer, L"%u:%02u %s", hour, time.wMinute,
               time.wHour >= 12 ? L"PM" : L"AM");
    return buffer;
}
