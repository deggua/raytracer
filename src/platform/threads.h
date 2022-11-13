#include <stdbool.h>
#include <stddef.h>

typedef struct Thread Thread;

Thread* Thread_New(void);
void    Thread_Delete(Thread* thread);

bool Thread_Spawn(Thread* thread, void (*entry)(void* arg), void* thread_arg);
bool Thread_Set_StackSize(Thread* thread, size_t stack_size);
void Thread_Join(Thread* thread);
