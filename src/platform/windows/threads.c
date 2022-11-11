#include "platform/threads.h"

#include <stdlib.h>
#include <windows.h>

typedef struct Thread {
    HANDLE thread_handle;
    size_t stack_size;
} Thread;

Thread* Thread_New(void)
{
    Thread* thread     = calloc(1, sizeof(Thread));
    thread->stack_size = 1 * 1024 * 1024;

    return thread;
}

void Thread_Delete(Thread* thread)
{
    free(thread);
}

bool Thread_Spawn(Thread* thread, void (*entry)(void* arg), void* thread_arg)
{
    // TODO: there's probably a better way, not sure how exactly though
    // should work as long as it doesn't break the ABI (e.g. returns on the stack)
    LPTHREAD_START_ROUTINE win_entry = (LPTHREAD_START_ROUTINE)entry;

    thread->thread_handle = CreateThread(NULL, thread->stack_size, win_entry, thread_arg, 0, NULL);

    if (thread->thread_handle == NULL) {
        return false;
    }

    return true;
}

void Thread_Set_StackSize(Thread* thread, size_t stack_size)
{
    thread->stack_size = stack_size;
}

void Thread_Join(Thread* thread)
{
    WaitForSingleObject(thread->thread_handle, INFINITE);
}
