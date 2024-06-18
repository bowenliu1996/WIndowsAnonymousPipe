
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>

using namespace std;

void RedirectCoutToConsole()
{
    // 打开控制台标准输出
    FILE* pConsole;
    freopen_s(&pConsole, "CONOUT$", "w", stdout);
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        /* * * 子进程 * * */
        RedirectCoutToConsole();
        cout << "Child Process" << endl;

        // 从匿名管道读数据
        CHAR szBuffer[16] = { 0 };
        ReadFile(GetStdHandle(STD_INPUT_HANDLE), szBuffer, sizeof(szBuffer), nullptr, nullptr);
        cout << "Received from parent: " << szBuffer << endl;

        // 向匿名管道写数据
        DWORD bytesWritten;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "message", 7, &bytesWritten, nullptr);
    }
    else
    {
        /* * * 父进程 * * */
        cout << "Parent Process" << endl;

        HANDLE hPipeReadToChild, hPipeWriteToChild;
        HANDLE hPipeReadFromChild, hPipeWriteFromChild;

        // 填充安全属性结构
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

        // 创建两个匿名管道
        if (!CreatePipe(&hPipeReadToChild, &hPipeWriteToChild, &sa, 0))
        {
            cout << "Create pipe to child failed!" << endl;
            return 1;
        }

        if (!CreatePipe(&hPipeReadFromChild, &hPipeWriteFromChild, &sa, 0))
        {
            cout << "Create pipe from child failed!" << endl;
            return 1;
        }

        // 初始化进程信息结构
        PROCESS_INFORMATION pi = { 0 };

        // 填充启动信息结构
        STARTUPINFOA si = { sizeof(si) };
        si.hStdInput = hPipeReadToChild;
        si.hStdOutput = hPipeWriteFromChild;
        si.dwFlags = STARTF_USESTDHANDLES;

        // 准备子进程的命令行参数
        char param[1024];
        sprintf_s(param, "%s %s", argv[0], "test");

        // 创建子进程
        if (!CreateProcessA(nullptr, param, nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi))
        {
            CloseHandle(hPipeReadToChild);
            CloseHandle(hPipeWriteToChild);
            CloseHandle(hPipeReadFromChild);
            CloseHandle(hPipeWriteFromChild);
            cout << "Create child process failed!" << endl;
            return 2;
        }

        // 关闭不必要的句柄
        CloseHandle(hPipeReadToChild);
        CloseHandle(hPipeWriteFromChild);

        // 向子进程写数据
        DWORD bytesWritten;
        WriteFile(hPipeWriteToChild, "ezhchai", 7, &bytesWritten, nullptr);
        CloseHandle(hPipeWriteToChild); // 关闭写句柄，通知子进程没有更多数据

        // 从子进程读数据
        CHAR szBuffer[256] = { 0 };
        DWORD bytesRead;
        ReadFile(hPipeReadFromChild, szBuffer, sizeof(szBuffer), &bytesRead, nullptr);
        szBuffer[bytesRead] = '\0'; // 确保字符串终止
        cout << "Received from child: " << szBuffer << endl;

        CloseHandle(hPipeReadFromChild);
    }

    system("pause");
    return 0;
}