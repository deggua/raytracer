#include "platform/threads.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct Thread {
    pthread_t      thread;
    pthread_attr_t thread_attr;
} Thread;

typedef struct ThreadArg {
    void* user_arg;
    void (*user_func)(void* arg);
} ThreadArg;

intern void* Thread_EntryWrapper(void* arg)
{
    ThreadArg* arg_wrapper = (ThreadArg*)arg;
    arg_wrapper->user_func(arg_wrapper->user_arg);
    free(arg);
    return NULL;
}

Thread* Thread_New(void)
{
    Thread* thread = (Thread*)calloc(1, sizeof(Thread));
    if (thread == NULL) {
        return NULL;
    }

    if (pthread_attr_init(&thread->thread_attr)) {
        free(thread);
        return NULL;
    }

    if (pthread_attr_setstacksize(&thread->thread_attr, 1 * 1024 * 1024)) {
        free(thread);
        return NULL;
    }

    return thread;
}

void Thread_Delete(Thread* thread)
{
    // NOTE: we assume this can't fail, there probably isn't anything meaningful
    // the user can do and I don't think this can realistically fail
    pthread_attr_destroy(&thread->thread_attr);
    free(thread);
}

bool Thread_Spawn(Thread* thread, void (*entry_point)(void* arg), void* thread_arg)
{
    ThreadArg* arg_wrapper = (ThreadArg*)malloc(sizeof(*arg_wrapper));
    if (arg_wrapper == NULL) {
        return false;
    }

    arg_wrapper->user_arg  = thread_arg;
    arg_wrapper->user_func = entry_point;

    if (pthread_create(&thread->thread, &thread->thread_attr, Thread_EntryWrapper, arg_wrapper)) {
        free(arg_wrapper);
        return false;
    }

    return true;
}

bool Thread_Set_StackSize(Thread* thread, size_t stack_size)
{
    if (pthread_attr_setstacksize(&thread->thread_attr, stack_size)) {
        return false;
    }

    return true;
}

void Thread_Join(Thread* thread)
{
    pthread_join(thread->thread, NULL);
}

void Thread_Kill(Thread* thread)
{
    // TODO: change to pthread_kill
    pthread_cancel(thread->thread);
}
