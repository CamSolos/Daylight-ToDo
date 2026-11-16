#pragma once

#include "Task.h"
#include <vector>
#include <string>

class TaskManager {
public:
    int NextId() const;
    const std::vector<Task>& GetTasks() const;
    Task* FindTask(int id);
    const Task* FindTask(int id) const;

    void AddTask(const std::wstring& description, const SYSTEMTIME& date,
                 const SYSTEMTIME& time);
    void UpdateTask(int id, const std::wstring& description,
                    const SYSTEMTIME& date, const SYSTEMTIME& time);
    void DeleteTask(int id);
    void SetCompleted(int id, bool completed);
    void ReplaceTasks(std::vector<Task> tasks);

private:
    std::vector<Task> m_tasks;
};
