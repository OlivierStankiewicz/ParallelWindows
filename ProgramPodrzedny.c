#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_THREADS 1024

typedef struct {
    DWORD threadId;
    HANDLE hThread;
    int durationSeconds;
} ThreadInfo;

ThreadInfo activeThreads[MAX_THREADS];
int numThreads = 0;

// Globalny licznik wątków
DWORD nextThreadId = 1;

void printMenu() {
    printf("\nMenu:\n");
    printf("a. Utworz nowy watek\n");
    printf("b. Usun wybrany watek\n");
    printf("c. Zmien priorytet wybranego watku\n");
    printf("d. Wyswietl liste watkow\n");
    printf("e. Zakoncz dany proces\n");
    printf("Choose option: ");
}

unsigned int __stdcall threadFunction(void* arg) {
    ThreadInfo* threadInfo = (ThreadInfo*)arg;
    DWORD threadId = threadInfo->threadId;
    int durationSeconds = threadInfo->durationSeconds;

    printf("Thread %lu started\n", threadId);
    Sleep(durationSeconds * 1000);
    printf("Thread %lu finished\n", threadId);

    for (int i = 0; i < numThreads; i++) {
        if (activeThreads[i].threadId == threadId) {
            for (int j = i; j < numThreads - 1; j++) {
                activeThreads[j] = activeThreads[j + 1];
            }
            numThreads--;
            break;
        }
    }

    return 0;
}

void createNewThread(int priority, int durationSeconds) {
    int threadPriority;
    switch(priority) {
        case 1:
            threadPriority = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        case 2:
            threadPriority = THREAD_PRIORITY_HIGHEST;
            break;
        case 3:
            threadPriority = THREAD_PRIORITY_NORMAL;
            break;
        case 4:
            threadPriority = THREAD_PRIORITY_IDLE;
            break;
        default:
            printf("Invalid priority\n");
            return;
    }
    
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, threadFunction, &activeThreads[numThreads], 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create thread\n");
        return;
    }

    activeThreads[numThreads].threadId = nextThreadId;
    activeThreads[numThreads].hThread = hThread;
    activeThreads[numThreads].durationSeconds = durationSeconds;
    numThreads++;
    nextThreadId++;

    if (!SetThreadPriority(hThread, threadPriority)) {
        printf("Failed to set thread priority (%lu)\n", GetLastError());
        CloseHandle(hThread);
        return;
    }
    printf("Thread %lu created successfully\n", nextThreadId - 1);
}

void deleteThread(DWORD threadId) {
    for (int i = 0; i < numThreads; i++) {
        if (activeThreads[i].threadId == threadId) {
            HANDLE hThread = activeThreads[i].hThread;
            if (!TerminateThread(hThread, 0)) {
                printf("Failed to terminate thread (%lu)\n", GetLastError());
                return;
            }
            printf("Thread with ID %lu terminated successfully\n", threadId);
            CloseHandle(hThread);

            for (int j = i; j < numThreads - 1; j++) {
                activeThreads[j] = activeThreads[j + 1];
            }
            numThreads--;
            return;
        }
    }
    printf("Thread with ID %lu not found\n", threadId);
}

void listThreads(DWORD processId) {
    printf("\nActive Thread IDs for process with PID %lu:\n", processId);
    for (int i = 0; i < numThreads; i++) {
        printf("%lu\n", activeThreads[i].threadId);
    }
}

void changeThreadPriority(DWORD threadId, int priority) {
    HANDLE hThread = NULL;
    int threadPriority;

    for (int i = 0; i < numThreads; i++) {
        if (activeThreads[i].threadId == threadId) {
            hThread = activeThreads[i].hThread;
            break;
        }
    }

    if (hThread == NULL) {
        printf("Thread with ID %lu not found\n", threadId);
        return;
    }

    switch(priority) {
        case 1:
            threadPriority = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        case 2:
            threadPriority = THREAD_PRIORITY_HIGHEST;
            break;
        case 3:
            threadPriority = THREAD_PRIORITY_NORMAL;
            break;
        case 4:
            threadPriority = THREAD_PRIORITY_IDLE;
            break;
        default:
            printf("Invalid priority\n");
            return;
    }

    if (!SetThreadPriority(hThread, threadPriority)) {
        printf("Failed to set thread priority (%lu)\n", GetLastError());
        return;
    }

    printf("Priority for thread %lu changed successfully\n", threadId);
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    int priority, durationSeconds;
    DWORD processId = GetCurrentProcessId();
    DWORD threadId;

    while (1) {
        printMenu();
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        switch (command[0]) {
            case 'a':
                printf("1. REALTIME_PRIORITY_CLASS\n");
                printf("2. HIGH_PRIORITY_CLASS\n");
                printf("3. NORMAL_PRIORITY_CLASS\n");
                printf("4. IDLE_PRIORITY_CLASS\n");
                printf("Choose option: ");
                scanf("%d", &priority);
                getchar();
                printf("Enter duration in seconds: ");
                scanf("%d", &durationSeconds);
                getchar();
                createNewThread(priority, durationSeconds);
                break;
            case 'b':
                printf("Enter Thread ID to delete: ");
                scanf("%lu", &threadId);
                getchar();
                deleteThread(threadId);
                break;
            case 'c':
                printf("Enter Thread ID: ");
                scanf("%lu", &threadId);
                getchar();
                printf("1. REALTIME_PRIORITY_CLASS\n");
                printf("2. HIGH_PRIORITY_CLASS\n");
                printf("3. NORMAL_PRIORITY_CLASS\n");
                printf("4. IDLE_PRIORITY_CLASS\n");
                printf("Choose priority: ");
                scanf("%d", &priority);
                getchar();
                changeThreadPriority(threadId, priority);
                break;
            case 'd':
                listThreads(processId);
                break;
            case 'e':
                return 0;
            default:
                printf("Invalid option\n");
                break;
        }
    }
    return 0;
}
