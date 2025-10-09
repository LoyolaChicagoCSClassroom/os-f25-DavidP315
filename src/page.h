#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>


struct ppage {
    struct ppage *next;
    struct ppage *prev;
    void *physical_addr;
};


#define NUM_PHYSICAL_PAGES 128
#define PAGE_SIZE_MB       2
#define PAGE_SIZE_BYTES    (PAGE_SIZE_MB * 1024 * 1024)

extern struct ppage *free_page_list;

void init_pfa_list(void);
struct ppage *allocate_physical_pages(unsigned int npages);
void free_physical_pages(struct ppage *ppage_list);

void print_pfa_state(void);

#endif
