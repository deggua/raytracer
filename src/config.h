#if defined(_WIN32)
#    define TARGET_WINDOWS
#    define _CRT_SECURE_NO_WARNINGS
#elif defined(__linux__)
#    define TARGET_LINUX
#endif
