/* ==================== syscalls.c ==================== */
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdint.h>

extern uint32_t _Min_Stack_Size;
/* ====================== Heap (malloc / free) ====================== */

caddr_t _sbrk(int incr)
{
    extern char _end;
    extern char _estack;

    static char *heap_end = NULL;
    char *prev_heap_end;

    if (heap_end == NULL)
        heap_end = &_end;

    prev_heap_end = heap_end;

    if((uintptr_t)heap_end + (uintptr_t)incr > (uintptr_t)&_estack - _Min_Stack_Size)
    {
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR; 
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}


int _write(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;


    return len;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
