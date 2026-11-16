# Daylight To-Do

A small Windows desktop to-do planner written in C++17. The app uses native Windows controls, so it does **not** require Qt, wxWidgets, FLTK, a package manager, or a third-party debugger.

## Features

- Add tasks with a description, date, and time.
- The date picker starts at today and does not allow a date before today.
- The application checks dates again before adding or editing a task.
- Check a task to mark it complete; completed task text is crossed out.
- Edit and delete tasks.
- Filter the list by all, pending, or completed tasks.
- Automatically save tasks and restore them the next time the app opens.
- Task data is stored in `%LOCALAPPDATA%\DaylightTodo\tasks.txt`.

## What you need

- Windows 10 or Windows 11
- Visual Studio 2022 Community (or Professional/Enterprise)
- During Visual Studio installation, select **Desktop development with C++**. This installs the MSVC compiler, Windows SDK, and Visual Studio's built-in debugger.

No separate debugger or GUI framework is required.

## Open and run in Visual Studio 2022

1. On the GitHub page, select **Code** and then **Download ZIP**. Alternatively, use Git to clone this repository.
2. Extract the ZIP to a normal folder, such as `Documents\Daylight-ToDo`.
3. Open the extracted folder and double-click `Daylight-ToDo.sln`.
4. Visual Studio opens the solution. If prompted to retarget the Windows SDK, accept the suggested installed SDK.
5. In the toolbar, choose **Debug** and **x64**.
6. Press **Ctrl+Shift+B** (Build > Build Solution). Wait for “Build succeeded”.
7. Press **F5** to run with Visual Studio's built-in debugger, or **Ctrl+F5** to run without attaching the debugger.
8. The Daylight window should open. Add a task to test it.

## Use the app

- Type a task description.
- Choose a date and time. Dates before today are rejected; today and future dates are allowed.
- Click **Add Task**.
- Tick a task's checkbox to complete it. Its text will be crossed out.
- Select a task and click **Edit selected** to load it into the fields. Change the details and click **Save Changes**.
- Click **Cancel** to leave edit mode.
- Select a task and click **Delete selected** to remove it.
- Use the filter at the top-right of the task list to show all, pending, or completed tasks.

## Build output

Visual Studio writes the executable to `bin\x64\Debug\Daylight-ToDo.exe` or `bin\x64\Release\Daylight-ToDo.exe`, depending on the selected configuration.

## Data and privacy

Task data is stored locally in the current Windows user's Local AppData folder. It is not uploaded to GitHub or sent to a server. Deleting the app folder does not automatically delete saved tasks; remove `%LOCALAPPDATA%\DaylightTodo\tasks.txt` to reset the list.

## Troubleshooting

- **Visual Studio says a C++ toolset is missing:** Open Visual Studio Installer, choose Modify, and add **Desktop development with C++**.
- **The solution asks about a Windows SDK:** Install a Windows 10 or Windows 11 SDK through Visual Studio Installer, then reopen the solution.
- **The build fails:** Check that the startup project is `Daylight-ToDo` and the toolbar is set to `x64`.

## License

Personal project. Add a license file before redistributing under specific terms.
