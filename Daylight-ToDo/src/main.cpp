#include "Task.h"
#include "TaskManager.h"
#include "FileHandler.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cwchar>

#pragma comment(lib, "Comctl32.lib")

namespace {
constexpr int ID_DESCRIPTION = 1001;
constexpr int ID_DATE = 1002;
constexpr int ID_TIME = 1003;
constexpr int ID_FILTER = 1004;
constexpr int ID_ADD = 1005;
constexpr int ID_EDIT = 1006;
constexpr int ID_CANCEL = 1007;
constexpr int ID_DELETE = 1008;
constexpr int ID_LIST = 1009;

const COLORREF COLOR_BACKGROUND = RGB(244, 247, 253);
const COLORREF COLOR_NAVY = RGB(29, 43, 68);
const COLORREF COLOR_TEXT = RGB(39, 52, 74);
const COLORREF COLOR_MUTED = RGB(112, 126, 148);
const COLORREF COLOR_BLUE = RGB(78, 105, 225);
const COLORREF COLOR_GREEN = RGB(36, 142, 105);
const COLORREF COLOR_RED = RGB(186, 78, 78);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);

std::wstring GetTaskDataPath() {
    wchar_t localAppData[MAX_PATH]{};
    DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    std::filesystem::path folder;
    if (length > 0 && length < MAX_PATH) {
        folder = std::filesystem::path(localAppData) / L"DaylightTodo";
    } else {
        folder = std::filesystem::current_path() / L"DaylightTodoData";
    }
    std::filesystem::create_directories(folder);
    return (folder / L"tasks.txt").wstring();
}

SYSTEMTIME GetToday() {
    SYSTEMTIME value{};
    GetLocalTime(&value);
    value.wHour = value.wMinute = value.wSecond = value.wMilliseconds = 0;
    return value;
}

class TodoWindow {
public:
    explicit TodoWindow(HINSTANCE instance) : m_instance(instance) {}

    bool Create(int showCommand) {
        INITCOMMONCONTROLSEX controls{ sizeof(controls), ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES };
        if (!InitCommonControlsEx(&controls)) return false;

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = m_instance;
        windowClass.lpszClassName = L"DaylightTodoMainWindow";
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        windowClass.hbrBackground = CreateSolidBrush(COLOR_BACKGROUND);
        if (!RegisterClassExW(&windowClass)) {
            DeleteObject(windowClass.hbrBackground);
            return false;
        }

        m_window = CreateWindowExW(0, windowClass.lpszClassName,
            L"Daylight | To-Do Planner", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1020, 780,
            nullptr, nullptr, m_instance, this);
        if (!m_window) {
            DeleteObject(windowClass.hbrBackground);
            return false;
        }
        ShowWindow(m_window, showCommand);
        UpdateWindow(m_window);
        return true;
    }

private:
    HINSTANCE m_instance{};
    HWND m_window{};
    HWND m_title{};
    HWND m_subtitle{};
    HWND m_descriptionLabel{};
    HWND m_dateLabel{};
    HWND m_timeLabel{};
    HWND m_description{};
    HWND m_date{};
    HWND m_time{};
    HWND m_filter{};
    HWND m_list{};
    HWND m_addButton{};
    HWND m_editButton{};
    HWND m_cancelButton{};
    HWND m_deleteButton{};
    HWND m_summary{};
    HFONT m_font{};
    HFONT m_boldFont{};
    HFONT m_titleFont{};
    HFONT m_strikeFont{};
    HBRUSH m_editBrush{};
    std::wstring m_dataPath;
    TaskManager m_tasks;
    int m_editingId{ -1 };
    bool m_refreshing{ false };

    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        TodoWindow* app = reinterpret_cast<TodoWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            auto* createInfo = reinterpret_cast<CREATESTRUCTW*>(lParam);
            app = static_cast<TodoWindow*>(createInfo->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
            app->m_window = window;
        }
        if (app) return app->HandleMessage(message, wParam, lParam);
        return DefWindowProcW(window, message, wParam, lParam);
    }

    HWND AddControl(const wchar_t* className, const wchar_t* text, DWORD style,
                    int controlId, HFONT font = nullptr, DWORD exStyle = 0) {
        HWND control = CreateWindowExW(exStyle, className, text,
            WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, m_window,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId)), m_instance, nullptr);
        if (control) SendMessageW(control, WM_SETFONT,
            reinterpret_cast<WPARAM>(font ? font : m_font), TRUE);
        return control;
    }

    HWND AddLabel(const wchar_t* text, HFONT font = nullptr) {
        return AddControl(L"STATIC", text, SS_LEFT, 0, font);
    }

    HWND AddButton(const wchar_t* text, int id) {
        return AddControl(L"BUTTON", text, BS_OWNERDRAW | WS_TABSTOP, id);
    }

    void CreateControls() {
        m_font = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        m_boldFont = CreateFontW(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        m_titleFont = CreateFontW(34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        LOGFONTW logFont{};
        GetObjectW(m_font, sizeof(logFont), &logFont);
        logFont.lfStrikeOut = TRUE;
        m_strikeFont = CreateFontIndirectW(&logFont);
        m_editBrush = CreateSolidBrush(COLOR_WHITE);

        m_title = AddLabel(L"My Tasks", m_titleFont);
        m_subtitle = AddLabel(L"Stay organized. Get things done.");
        m_descriptionLabel = AddLabel(L"TASK DESCRIPTION", m_boldFont);
        m_dateLabel = AddLabel(L"DATE", m_boldFont);
        m_timeLabel = AddLabel(L"TIME", m_boldFont);
        m_description = AddControl(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                   ID_DESCRIPTION);
        m_date = AddControl(DATETIMEPICK_CLASSW, L"", DTS_SHORTDATEFORMAT | WS_TABSTOP, ID_DATE);
        m_time = AddControl(DATETIMEPICK_CLASSW, L"", DTS_TIMEFORMAT | WS_TABSTOP, ID_TIME);
        m_filter = AddControl(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, ID_FILTER);
        SendMessageW(m_filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"All tasks"));
        SendMessageW(m_filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Pending"));
        SendMessageW(m_filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Completed"));
        SendMessageW(m_filter, CB_SETCURSEL, 0, 0);

        m_addButton = AddButton(L"+  Add Task", ID_ADD);
        m_editButton = AddButton(L"Edit selected", ID_EDIT);
        m_cancelButton = AddButton(L"Cancel", ID_CANCEL);
        ShowWindow(m_cancelButton, SW_HIDE);
        m_deleteButton = AddButton(L"Delete selected", ID_DELETE);
        m_summary = AddLabel(L"");

        m_list = AddControl(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                            WS_BORDER | WS_TABSTOP, ID_LIST, m_font, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(m_list,
            LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        AddColumn(L"TASK", 430, 0);
        AddColumn(L"DATE", 150, 1);
        AddColumn(L"TIME", 130, 2);
        AddColumn(L"STATUS", 130, 3);

        SYSTEMTIME today = GetToday();
        DateTime_SetSystemtime(m_date, GDT_VALID, &today);
        SYSTEMTIME now{};
        GetLocalTime(&now);
        DateTime_SetSystemtime(m_time, GDT_VALID, &now);
        SYSTEMTIME range[2]{};
        range[0] = today;
        range[1].wYear = 2099; range[1].wMonth = 12; range[1].wDay = 31;
        DateTime_SetRange(m_date, GDTR_MIN | GDTR_MAX, range);

        m_dataPath = GetTaskDataPath();
        LoadTasks();
        RefreshList();
    }

    void AddColumn(const wchar_t* heading, int width, int index) {
        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<LPWSTR>(heading);
        column.cx = width;
        column.iSubItem = index;
        ListView_InsertColumn(m_list, index, &column);
    }

    void Layout(int width, int height) {
        const int margin = 30;
        const int right = width - margin;
        const int rowWidth = width - margin * 2;
        MoveWindow(m_title, margin, 20, 450, 48, TRUE);
        MoveWindow(m_subtitle, margin + 2, 68, 500, 26, TRUE);

        MoveWindow(m_descriptionLabel, margin, 115, 300, 24, TRUE);
        MoveWindow(m_description, margin, 142, rowWidth - 390, 38, TRUE);
        MoveWindow(m_dateLabel, right - 365, 115, 100, 24, TRUE);
        MoveWindow(m_date, right - 365, 142, 170, 38, TRUE);
        MoveWindow(m_timeLabel, right - 180, 115, 100, 24, TRUE);
        MoveWindow(m_time, right - 180, 142, 150, 38, TRUE);
        MoveWindow(m_addButton, margin, 194, rowWidth, 44, TRUE);

        MoveWindow(m_filter, right - 205, 265, 205, 34, TRUE);
        MoveWindow(m_list, margin, 307, rowWidth, (std::max)(200, height - 420), TRUE);
        MoveWindow(m_summary, margin, height - 88, 420, 32, TRUE);
        MoveWindow(m_editButton, right - 390, height - 98, 150, 44, TRUE);
        MoveWindow(m_cancelButton, right - 225, height - 98, 95, 44, TRUE);
        MoveWindow(m_deleteButton, right - 120, height - 98, 120, 44, TRUE);
    }

    int SelectedTaskId() const {
        int index = ListView_GetNextItem(m_list, -1, LVNI_SELECTED);
        if (index < 0) return -1;
        LVITEMW item{};
        item.mask = LVIF_PARAM;
        item.iItem = index;
        if (!ListView_GetItem(m_list, &item)) return -1;
        return static_cast<int>(item.lParam);
    }

    void RefreshList() {
        if (!m_list) return;
        m_refreshing = true;
        ListView_DeleteAllItems(m_list);
        const int filter = static_cast<int>(SendMessageW(m_filter, CB_GETCURSEL, 0, 0));
        int visible = 0;
        int completed = 0;
        for (const Task& task : m_tasks.GetTasks()) {
            if (task.IsCompleted()) ++completed;
            if (filter == 1 && task.IsCompleted()) continue;
            if (filter == 2 && !task.IsCompleted()) continue;

            std::wstring description = task.GetDescription();
            LVITEMW item{};
            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.iItem = visible;
            item.pszText = description.data();
            item.lParam = task.GetId();
            int row = ListView_InsertItem(m_list, &item);
            std::wstring date = FormatDate(task.GetDate());
            std::wstring time = FormatTime(task.GetTime());
            std::wstring status = task.IsCompleted() ? L"Completed" : L"Pending";
            ListView_SetItemText(m_list, row, 1, date.data());
            ListView_SetItemText(m_list, row, 2, time.data());
            ListView_SetItemText(m_list, row, 3, status.data());
            ListView_SetCheckState(m_list, row, task.IsCompleted());
            ++visible;
        }
        const int total = static_cast<int>(m_tasks.GetTasks().size());
        std::wstring summary = std::to_wstring(total) + L" tasks     " +
            std::to_wstring(completed) + L" completed     " +
            std::to_wstring(total - completed) + L" remaining";
        SetWindowTextW(m_summary, summary.c_str());
        m_refreshing = false;
    }

    void LoadTasks() {
        try {
            std::vector<Task> loaded = FileHandler::Load(m_dataPath);
            m_tasks.ReplaceTasks(std::move(loaded));
        } catch (const std::exception& error) {
            ShowError(L"Saved tasks could not be loaded. Your current list is still usable.\n\n" +
                      ToWide(error.what()));
        }
    }

    std::wstring ToWide(const std::string& value) const {
        if (value.empty()) return {};
        int size = MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, nullptr, 0);
        if (size <= 1) return L"An unexpected error occurred.";
        std::wstring result(static_cast<size_t>(size), L'\0');
        MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, result.data(), size);
        result.pop_back();
        return result;
    }

    void SaveTasks() {
        try {
            FileHandler::Save(m_tasks.GetTasks(), m_dataPath);
        } catch (const std::exception& error) {
            ShowError(L"Could not save your tasks.\n\n" + ToWide(error.what()));
        }
    }

    void ShowError(const std::wstring& message) const {
        MessageBoxW(m_window, message.c_str(), L"Daylight To-Do", MB_OK | MB_ICONWARNING);
    }

    void ResetEditor() {
        m_editingId = -1;
        SetWindowTextW(m_addButton, L"+  Add Task");
        ShowWindow(m_cancelButton, SW_HIDE);
        SetWindowTextW(m_description, L"");
        SYSTEMTIME today = GetToday();
        DateTime_SetSystemtime(m_date, GDT_VALID, &today);
        SYSTEMTIME now{};
        GetLocalTime(&now);
        DateTime_SetSystemtime(m_time, GDT_VALID, &now);
        InvalidateRect(m_addButton, nullptr, TRUE);
    }

    void AddOrUpdateTask() {
        int length = GetWindowTextLengthW(m_description);
        std::wstring description(static_cast<size_t>(length) + 1, L'\0');
        GetWindowTextW(m_description, description.data(), length + 1);
        description.resize(static_cast<size_t>(length));
        SYSTEMTIME date{}, time{};
        if (DateTime_GetSystemtime(m_date, &date) != GDT_VALID ||
            DateTime_GetSystemtime(m_time, &time) != GDT_VALID) {
            ShowError(L"Please enter a valid date and time.");
            return;
        }
        try {
            if (m_editingId >= 0) {
                m_tasks.UpdateTask(m_editingId, description, date, time);
            } else {
                m_tasks.AddTask(description, date, time);
            }
            SaveTasks();
            ResetEditor();
            RefreshList();
        } catch (const std::exception& error) {
            ShowError(ToWide(error.what()));
        }
    }

    void EditSelectedTask() {
        const int id = SelectedTaskId();
        const Task* task = m_tasks.FindTask(id);
        if (!task) {
            ShowError(L"Select a task first.");
            return;
        }
        m_editingId = id;
        SetWindowTextW(m_description, task->GetDescription().c_str());
        DateTime_SetSystemtime(m_date, GDT_VALID, &task->GetDate());
        DateTime_SetSystemtime(m_time, GDT_VALID, &task->GetTime());
        SetWindowTextW(m_addButton, L"Save Changes");
        ShowWindow(m_cancelButton, SW_SHOW);
        InvalidateRect(m_addButton, nullptr, TRUE);
        SetFocus(m_description);
    }

    void DeleteSelectedTask() {
        const int id = SelectedTaskId();
        if (id < 0) {
            ShowError(L"Select a task first.");
            return;
        }
        if (MessageBoxW(m_window, L"Delete the selected task?", L"Confirm delete",
            MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        try {
            m_tasks.DeleteTask(id);
            if (m_editingId == id) ResetEditor();
            SaveTasks();
            RefreshList();
        } catch (const std::exception& error) {
            ShowError(ToWide(error.what()));
        }
    }

    void OnListItemChanged(const NMLISTVIEW* change) {
        if (m_refreshing || !change ||
            ((change->uOldState ^ change->uNewState) & LVIS_STATEIMAGEMASK) == 0) return;
        LVITEMW item{};
        item.mask = LVIF_PARAM;
        item.iItem = change->iItem;
        if (!ListView_GetItem(m_list, &item)) return;
        const bool checked = ListView_GetCheckState(m_list, change->iItem) != FALSE;
        try {
            m_tasks.SetCompleted(static_cast<int>(item.lParam), checked);
            SaveTasks();
            RefreshList();
        } catch (const std::exception& error) {
            ShowError(ToWide(error.what()));
        }
    }

    void DrawButton(const DRAWITEMSTRUCT* draw) {
        COLORREF color = COLOR_BLUE;
        if (draw->CtlID == ID_DELETE) color = COLOR_RED;
        else if (draw->CtlID == ID_CANCEL || draw->CtlID == ID_EDIT) color = RGB(225, 231, 242);
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(draw->hDC, &draw->rcItem, brush);
        DeleteObject(brush);

        SetBkMode(draw->hDC, TRANSPARENT);
        SetTextColor(draw->hDC, (draw->CtlID == ID_CANCEL || draw->CtlID == ID_EDIT)
            ? COLOR_TEXT : COLOR_WHITE);
        HFONT oldFont = static_cast<HFONT>(SelectObject(draw->hDC, m_boldFont));
        wchar_t text[128]{};
        GetWindowTextW(draw->hwndItem, text, 128);
        RECT textRect = draw->rcItem;
        DrawTextW(draw->hDC, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(draw->hDC, oldFont);
        if (draw->itemState & ODS_FOCUS) {
            RECT focusRect = draw->rcItem;
            InflateRect(&focusRect, -5, -5);
            DrawFocusRect(draw->hDC, &focusRect);
        }
    }

    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_CREATE:
            CreateControls();
            return 0;
        case WM_SIZE:
            if (m_window) Layout(LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_GETMINMAXINFO: {
            auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 900;
            info->ptMinTrackSize.y = 680;
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_ADD: AddOrUpdateTask(); return 0;
            case ID_EDIT: EditSelectedTask(); return 0;
            case ID_CANCEL: ResetEditor(); return 0;
            case ID_DELETE: DeleteSelectedTask(); return 0;
            case ID_FILTER:
                if (HIWORD(wParam) == CBN_SELCHANGE) RefreshList();
                return 0;
            }
            break;
        case WM_NOTIFY: {
            auto* header = reinterpret_cast<NMHDR*>(lParam);
            if (header->idFrom == ID_LIST && header->code == LVN_ITEMCHANGED) {
                OnListItemChanged(reinterpret_cast<NMLISTVIEW*>(lParam));
                return 0;
            }
            if (header->idFrom == ID_LIST && header->code == NM_CUSTOMDRAW) {
                auto* draw = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);
                if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
                if (draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                    LVITEMW item{};
                    item.mask = LVIF_PARAM;
                    item.iItem = static_cast<int>(draw->nmcd.dwItemSpec);
                    if (ListView_GetItem(m_list, &item)) {
                        const Task* task = m_tasks.FindTask(static_cast<int>(item.lParam));
                        if (task && task->IsCompleted()) {
                            draw->clrText = COLOR_MUTED;
                            SelectObject(draw->nmcd.hdc, m_strikeFont);
                            return CDRF_NEWFONT;
                        }
                    }
                    return CDRF_DODEFAULT;
                }
            }
            break;
        }
        case WM_DRAWITEM:
            DrawButton(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
            return TRUE;
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wParam);
            SetBkMode(dc, TRANSPARENT);
            HWND control = reinterpret_cast<HWND>(lParam);
            SetTextColor(dc, control == m_title ? COLOR_NAVY : COLOR_MUTED);
            return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
        }
        case WM_CTLCOLOREDIT: {
            HDC dc = reinterpret_cast<HDC>(wParam);
            SetTextColor(dc, COLOR_TEXT);
            SetBkColor(dc, COLOR_WHITE);
            return reinterpret_cast<LRESULT>(m_editBrush);
        }
        case WM_ERASEBKGND: {
            RECT rect{};
            GetClientRect(m_window, &rect);
            HBRUSH brush = CreateSolidBrush(COLOR_BACKGROUND);
            FillRect(reinterpret_cast<HDC>(wParam), &rect, brush);
            DeleteObject(brush);
            return 1;
        }
        case WM_CLOSE:
            SaveTasks();
            DestroyWindow(m_window);
            return 0;
        case WM_DESTROY:
            if (m_font) DeleteObject(m_font);
            if (m_boldFont) DeleteObject(m_boldFont);
            if (m_titleFont) DeleteObject(m_titleFont);
            if (m_strikeFont) DeleteObject(m_strikeFont);
            if (m_editBrush) DeleteObject(m_editBrush);
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(m_window, message, wParam, lParam);
    }
};
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    TodoWindow application(instance);
    if (!application.Create(showCommand)) {
        MessageBoxW(nullptr, L"Daylight could not create its window.",
                     L"Daylight To-Do", MB_OK | MB_ICONERROR);
        return 1;
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
