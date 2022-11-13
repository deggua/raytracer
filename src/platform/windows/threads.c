#include "platform/threads.h"

#include <stdlib.h>
#include <windows.h>

typedef struct Thread {
    HANDLE thread_handle;
    size_t stack_size;
} Thread;

typedef struct ThreadArg {
    void* user_arg;
    void (*user_func)(void* arg);
} ThreadArg;

static DWORD Thread_EntryWrapper(LPVOID arg)
{
    ThreadArg* arg_wrapper = arg;
    arg_wrapper->user_func(arg_wrapper->user_arg);
    free(arg);
    return 0;
}

Thread* Thread_New(void)
{
    Thread* thread = calloc(1, sizeof(Thread));
    if (thread == NULL) {
        return NULL;
    }

    thread->stack_size = 1 * 1024 * 1024;

    return thread;
}

void Thread_Delete(Thread* thread)
{
    free(thread);
}

bool Thread_Spawn(Thread* thread, void (*entry_point)(void* arg), void* thread_arg)
{
    ThreadArg* arg_wrapper = malloc(sizeof(*arg_wrapper));
    if (arg_wrapper == NULL) {
        return false;
    }

    arg_wrapper->user_arg  = thread_arg;
    arg_wrapper->user_func = entry_point;

    thread->thread_handle = CreateThread(NULL, thread->stack_size, Thread_EntryWrapper, (LPVOID)arg_wrapper, 0, NULL);

    if (thread->thread_handle == NULL) {
        free(arg_wrapper);
        return false;
    }

    return true;
}

bool Thread_Set_StackSize(Thread* thread, size_t stack_size)
{
    thread->stack_size = stack_size;

    return true;
}

void Thread_Join(Thread* thread)
{
    WaitForSingleObject(thread->thread_handle, INFINITE);
}
