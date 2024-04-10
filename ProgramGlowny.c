#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>

#define MAX_PROCESSES 1024
#define MAX_COMMAND_LENGTH 100
#define SUBPROCESS_PATH "./ProgramPodrzedny.exe"

typedef struct {
    DWORD pid;
    char path[MAX_COMMAND_LENGTH];
} ProcessInfo;

ProcessInfo processes[MAX_PROCESSES];
int numProcesses = 0;

void printMenu() {
    printf("\nMenu:\n");
    printf("1. Stworz nowy proces\n");
    printf("2. Zatrzymaj wybrany proces\n");
    printf("3. Wyswietl liste procesow\n");
    printf("4. Zamknij program\n");
    printf("Choose option: ");
}

void removeProcess(DWORD processId) {
    int i;
    for (i = 0; i < numProcesses; i++) {
        if (processes[i].pid == processId) {
            for (int j = i; j < numProcesses - 1; j++) {
                processes[j] = processes[j + 1];
            }
            numProcesses--;
            printf("Process with PID %lu stopped and removed from the list\n", processId);
            return;
        }
    }
    printf("Process with PID %lu not found\n", processId);
}

void stopProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        printf("Failed to open process (%lu)\n", GetLastError());
        return;
    }
    if (!TerminateProcess(hProcess, 0)) {
        printf("Failed to terminate process (%lu)\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }
    printf("Process with PID %lu stopped successfully\n", processId);
    CloseHandle(hProcess);

    removeProcess(processId);
}

void listProcesses() {
    printf("\nPID\tProcess Name\n");
    for (int i = 0; i < numProcesses; i++) {
        DWORD exitCode;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i].pid);
        if (hProcess != NULL) {
            GetExitCodeProcess(hProcess, &exitCode);
            CloseHandle(hProcess);
            if (exitCode != STILL_ACTIVE) {
                removeProcess(processes[i].pid);
                i--;
                continue;
            }
        }
        printf("%lu\t%s\n", processes[i].pid, processes[i].path);
    }
}

void closeAllProcesses() {
    for (int i = 0; i < numProcesses; i++) {
        stopProcess(processes[i].pid);
    }
}

int createNewProcess(int priority) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;

    DWORD priorityClass;
    switch(priority) {
        case 1:
            priorityClass = REALTIME_PRIORITY_CLASS;
            break;
        case 2:
            priorityClass = HIGH_PRIORITY_CLASS;
            break;
        case 3:
            priorityClass = NORMAL_PRIORITY_CLASS;
            break;
        case 4:
            priorityClass = IDLE_PRIORITY_CLASS;
            break;
        default:
            printf("Invalid priority\n");
            return -1;
    }

    if (!CreateProcess(NULL, SUBPROCESS_PATH, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | priorityClass, NULL, NULL, &si, &pi)) {
        printf("Failed to create process (%lu)\n", GetLastError());
        return -1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    processes[numProcesses].pid = pi.dwProcessId;
    strcpy(processes[numProcesses].path, SUBPROCESS_PATH);
    numProcesses++;

    printf("Process created successfully\n");

    return 0;
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    DWORD pid;
    while (1) {
        printMenu();
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        switch (atoi(command)) {
            case 1:
                printf("1. REALTIME_PRIORITY_CLASS\n");
                printf("2. HIGH_PRIORITY_CLASS\n");
                printf("3. NORMAL_PRIORITY_CLASS\n");
                printf("4. IDLE_PRIORITY_CLASS\n");
                printf("Choose priority: ");
                int priority;
                scanf("%d", &priority);
                getchar();
                if (createNewProcess(priority) == -1) {
                    printf("Failed to create process\n");
                }
                break;
            case 2:
                printf("Enter PID of the process to stop: ");
                scanf("%lu", &pid);
                stopProcess(pid);
                break;
            case 3:
                listProcesses();
                break;
            case 4:
                closeAllProcesses();
                return 0;
            default:
                printf("Invalid option\n");
                break;
        }
    }
    return 0;
}
