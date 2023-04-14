#include "MKL25Z4.h"
#include "uart.h"
#include <sys/stat.h>
#include <sys/types.h>

extern char _end;

void *_sbrk(int incr) {
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;
    if (heap == NULL) heap = (unsigned char *)&_end;
    prev_heap = heap;
    heap += incr;
    return prev_heap;
}

int _fstat(int fd, struct stat *st) {
    if (fd < 0) return -1;
    st->st_mode = S_IFCHR;
    return 0;
}

int _close(int fd) {
    (void)fd;
    return -1;
}

int _isatty(int fd) {
    (void)fd;
    return 1;
}

int _lseek(int fd, int ptr, int dir) {
    (void)fd, (void)ptr, (void)dir;
    return 0;
}

int _write(int fd, char *ptr, int len) {
    (void)fd, (void)ptr, (void)len;
    if (fd == 1) {
        uart_write_buf(UART1_BASE_PTR, ptr, (size_t)len);
        return len;
    }
    return -1;
}

int _read(int fd, char *ptr, int len) {
    (void)fd, (void)ptr, (void)len;
    return -1;
}
