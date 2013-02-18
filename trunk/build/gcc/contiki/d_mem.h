#ifndef D_MEM_H_
#define D_MEM_H_

#include <stdint.h>
#define D_MEM_HEAP_SIZE 1500

// align on this number of byte boundarie#s
#define D_MEM_ALIGN   2
#define D_MEM_LEN     0x0fff
#define D_MEM_FLAGS   0x7000
#define D_MEM_INUSE   0x8000

extern uint8_t heap[D_MEM_HEAP_SIZE];
typedef uint16_t bndrt_t;

void d_malloc_init();
void* d_malloc(uint16_t sz);
void d_free(void *ptr);
uint16_t d_malloc_freespace();

void* d_realloc(void* ptr, uint16_t size);

uint16_t getMemUsage();

#endif // D_MEM_H_
