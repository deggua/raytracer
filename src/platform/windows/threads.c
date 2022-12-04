#include "platform/threads.h"

#include <stdlib.h>
#include <windows.h>

typedef struct ThreadArg {
    void* user_arg;
    void (*user_func)(void* arg);
} ThreadArg;

typedef struct Thread {
    HANDLE     handle;
    size_t     stack_size;
    ThreadArg* arg;
} Thread;

intern DWORD Thread_EntryWrapper(LPVOID arg)
{
    Thread*    thread = arg;
    ThreadArg* args   = thread->arg;

    args->user_func(args->user_arg);

    thread->arg = NULL;
    free(args);

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

    thread->arg    = arg_wrapper;
    thread->handle = CreateThread(NULL, thread->stack_size, Thread_EntryWrapper, thread, 0, NULL);

    if (thread->handle == NULL) {
        thread->arg = NULL;
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
    WaitForSingleObject(thread->handle, INFINITE);
}

void Thread_Kill(Thread* thread)
{
    TerminateThread(thread, 0);
    free(thread->arg);
}
