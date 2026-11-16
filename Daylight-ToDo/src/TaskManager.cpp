#include "TaskManager.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

int TaskManager::NextId() const {
    int highestId = 0;
    for (const Task& task : m_tasks) highestId = (std::max)(highestId, task.GetId());
    return highestId + 1;
}

const std::vector<Task>& TaskManager::GetTasks() const { return m_tasks; }

Task* TaskManager::FindTask(int id) {
    for (Task& task : m_tasks) if (task.GetId() == id) return &task;
    return nullptr;
}

const Task* TaskManager::FindTask(int id) const {
    for (const Task& task : m_tasks) if (task.GetId() == id) return &task;
    return nullptr;
}

void TaskManager::AddTask(const std::wstring& description, const SYSTEMTIME& date,
                          const SYSTEMTIME& time) {
    if (IsDateBeforeToday(date)) {
        throw std::invalid_argument("Past dates are not allowed. Choose today or a future date.");
    }
    m_tasks.emplace_back(NextId(), description, date, time);
}

void TaskManager::UpdateTask(int id, const std::wstring& description,
                             const SYSTEMTIME& date, const SYSTEMTIME& time) {
    Task* task = FindTask(id);
    if (!task) throw std::invalid_argument("That task could not be found.");
    if (IsDateBeforeToday(date)) {
        throw std::invalid_argument("Past dates are not allowed. Choose today or a future date.");
    }
    // Validate everything before changing the existing task.
    Task checked(id, description, date, time, task->IsCompleted());
    *task = std::move(checked);
}

void TaskManager::DeleteTask(int id) {
    const auto oldSize = m_tasks.size();
    m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
        [id](const Task& task) { return task.GetId() == id; }), m_tasks.end());
    if (m_tasks.size() == oldSize) throw std::invalid_argument("That task could not be found.");
}

void TaskManager::SetCompleted(int id, bool completed) {
    Task* task = FindTask(id);
    if (!task) throw std::invalid_argument("That task could not be found.");
    task->SetCompleted(completed);
}

void TaskManager::ReplaceTasks(std::vector<Task> tasks) {
    for (size_t i = 0; i < tasks.size(); ++i) {
        for (size_t j = i + 1; j < tasks.size(); ++j) {
            if (tasks[i].GetId() == tasks[j].GetId()) {
                throw std::invalid_argument("The saved task file contains duplicate IDs.");
            }
        }
    }
    m_tasks = std::move(tasks);
}
