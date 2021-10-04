#include <stdio.h>
#include <stdlib.h>

#ifndef empty
#define empty() do{}while(0)
#endif

#ifndef _MEMORY_LOGGER_
#define _MEMORY_LOGGER_
#define MAX_POINTER_COUNT 100000
void memoryLoggerInit(FILE*);
int memoryLoggerAtExitHook(int);
void* m_malloc(size_t, char*);
void* m_calloc(size_t, size_t, char*);
void* m_realloc(void*, size_t);
void m_free(void*);
void (printMemoryLogs)();

#if LOG_MEMORY
#define malloc(size, name) m_malloc((size),(name))
#define calloc(count, size, name) m_calloc((count),(size),(name))
#define realloc(pointer, size) m_realloc((pointer),(size))
#define free(pointer) m_free((pointer))
#define printMemoryLogs() (printMemoryLogs)()
#define atexit(fun) (atexit)((fun))
#else
#define malloc(size, name) (malloc)((size))
#define calloc(count, size, name) (calloc)((count),(size))
#define realloc(pointer,size) (realloc)((pointer),(size))
#define free(pointer) (free)((pointer))
#define printMemoryLogs() empty()
#define atexit(fun) empty()
#endif
#endif