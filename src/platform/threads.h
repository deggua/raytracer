#include <stdbool.h>

#include "common/common.h"

typedef struct Thread Thread;

Thread* Thread_New(void);
void    Thread_Delete(Thread* thread);

bool Thread_Spawn(Thread* thread, void (*entry)(void* arg), void* thread_arg);
void Thread_Set_StackSize(Thread* thread, size_t stack_size);
void Thread_Join(Thread* thread);
