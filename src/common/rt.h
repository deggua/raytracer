typedef enum RTCode
{
    RT_SUCCESS = 0, // Success
    RT_OOM,         // Ran out of memory
    RT_BAD_STATE,   // Reached a bad state which was unrecoverable
} RTCode;
