#include "platform/threads.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct Thread {
    pthread_t      thread;
    pthread_attr_t thread_attr;
} Thread;

Thread* Thread_New(void)
{
    Thread* thread = calloc(1, sizeof(Thread));
    pthread_attr_init(&thread->thread_attr);
    pthread_attr_setstacksize(&thread->thread_attr, 1 * 1024 * 1024);
    return thread;
}

void Thread_Delete(Thread* thread)
{
    pthread_attr_destroy(&thread->thread_attr);
    free(thread);
}

bool Thread_Spawn(Thread* thread, void (*entry)(void* arg), void* thread_arg)
{
    // TODO: there's probably a better way, not sure how exactly though
    // should work as long as it doesn't break the ABI (e.g. returns on the stack)
    void* (*linux_entry)(void* arg) = (void* (*)(void*))entry;

    pthread_create(&thread->thread, &thread->thread_attr, linux_entry, thread_arg);

    if (thread->thread_handle != 0) {
        return false;
    }

    return true;
}

void Thread_Set_StackSize(Thread* thread, size_t stack_size)
{
    pthread_attr_setstacksize(&thread->thread_attr, stack_size);
}

void Thread_Join(Thread* thread)
{
    pthread_join(thread->thread);
}
