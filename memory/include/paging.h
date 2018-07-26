/*
 * Copyright (c) 2018 Jie Zheng
 */

#ifndef _PAGING_H
#define _PAGING_H
#include<memory/include/physical_memory.h>
/*
 * page directory entry in 32-bit mode
 */
struct pde32 {
    uint32_t present:1;
    uint32_t write_permission:1;
    uint32_t supervisor_permission:1;
    uint32_t page_writethrough:1;
    uint32_t page_cachedisable:1;
    uint32_t accessed:1;
    uint32_t reserved0:1;
    uint32_t page_size:1;
    uint32_t reserved1:4;
    uint32_t pt_frame:20;
}__attribute__((packed));

/*
 * page table entry in 32-bit
 */
struct pte32 {
    uint32_t present:1;
    uint32_t write_permission:1;
    uint32_t supervisor_permission:1;
    uint32_t page_writethrough:1;
    uint32_t page_cachedisable:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t pat:1;// page attribute table
    uint32_t global:1;
    uint32_t reserved:3;
    uint32_t pg_frame:20;
}__attribute__((packed));

#define PDE32_TO_DWORD(entry) (*(uint32_t*)(entry))
#define PTE32_TO_DWORD(entry) PDE32_TO_DWORD(entry)

#define PDE32_PTR(addr) ((struct pde32*)(addr))
#define PTE32_PTR(addr) ((struct pte32*)(addr))

#define FREE_PAGE_BITMAP_SIZE  ((1 << 20) >> 3)
#define PHYSICAL_MEMORY_TO_PAGE_FRAME(mem) (((uint32_t)(mem)) >> 12)

#define AT_BYTE(fn) ((fn) >> 3)
#define AT_BIT(fn) ((fn) & 0x7)

#define IS_PAGE_FREE(pg_fn) ({\
    uint32_t _byte_index = AT_BYTE((pg_fn)); \
    uint8_t _bit_index = AT_BIT((pg_fn)); \
    !((free_page_bitmap[_byte_index] >> _bit_index) & 0x1); \
})

#define MAKR_PAGE_AS_FREE(pg_fn) {\
    uint32_t _byte_index = AT_BYTE((pg_fn)); \
    uint8_t _bit_index = AT_BIT((pg_fn)); \
    uint8_t _mask = 1 << _bit_index; \
    _mask = ~_mask; \
    free_page_bitmap[_byte_index] &= _mask; \
}
#define MARK_PAGE_AS_OCCUPIED(pg_fn) { \
    uint32_t _byte_index = AT_BYTE((pg_fn)); \
    uint8_t _bit_index = AT_BIT((pg_fn)); \
    uint8_t _mask = 1 << _bit_index; \
    free_page_bitmap[_byte_index] |= _mask; \
}
#define SEARCH_FREE_PAGE(begin_index) ({\
    int32_t _idx = 0; \
    int32_t _target = -1; \
    for (_idx = (begin_index); _idx < FREE_PAGE_BITMAP_SIZE << 3; _idx++) { \
        if (IS_PAGE_FREE(_idx)) { \
            _target = _idx; \
            break; \
        } \
    } \
    _target; \
})

#define UPPER_MEMORY_PAGE_INDEX  PHYSICAL_MEMORY_TO_PAGE_FRAME(0x100000)

#define PAGE_PERMISSION_READ_ONLY 0x0
#define PAGE_PERMISSION_READ_WRITE 0x1

#define PAGE_PERMISSION_USER 0x1
#define PAGE_PERMISSION_SUPERVISOR 0x0

#define PAGE_WRITETHROUGH 0x1
#define PAGE_WRITEBACK 0x0

#define PAGE_CACHE_DISABLED 0x1
#define PAGE_CACHE_ENABLED 0x0

void paging_init(void);
#endif