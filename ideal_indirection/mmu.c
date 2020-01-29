/**
* Ideal Indirection Lab
* CS 241 - Fall 2018
*/

#include "mmu.h"
#include "tlb.h"
#include "segments.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static size_t current_pid = 0;

mmu *mmu_create() {
    mmu *my_mmu = calloc(1, sizeof(mmu));
    my_mmu->tlb = tlb_create();
    return my_mmu;
}

page_table_entry *mmu_get_pte(mmu *this, uintptr_t virtual_address, size_t pid)
{
    if(!address_in_segmentations(this->segmentations[pid], virtual_address))
    {
        mmu_raise_segmentation_fault(this);
        return NULL;
    }

    if(current_pid != pid)
    {
        tlb_flush(&(this->tlb));
        current_pid = pid;
    }
    
    page_table_entry* physical = NULL;
    physical = tlb_get_pte(&(this -> tlb), virtual_address>>12);

    if(!physical)
    {
        mmu_tlb_miss(this);
        unsigned int directory_index = virtual_address >> 22;
        page_directory_entry* pt_32 = &(this->page_directories[pid]->entries[directory_index]);
        unsigned int page_table_index = ((directory_index << 22) ^ virtual_address) >> 12;

        if(!(pt_32->present))
        {
            mmu_raise_page_fault(this);
            //fprintf(stderr, "page_table_original_address: %u\n", pt_32 -> base_addr);
            addr32 page_basical = ask_kernel_for_frame((page_table_entry*)pt_32);
            pt_32 -> base_addr = page_basical >> 12;
            read_page_from_disk((page_table_entry*)pt_32);
            pt_32->present = 1;

            page_table* real_page_table = (page_table*) get_system_pointer_from_pde(pt_32); 
            //fprintf(stderr, "page_table_address: %u\n", pt_32 -> base_addr);
            physical = &(real_page_table -> entries[page_table_index]);
            //fprintf(stderr, "physical_address: %p\n", physical);


            mmu_raise_page_fault(this);
            addr32 physical_base = ask_kernel_for_frame(physical);
            physical -> base_addr = physical_base >> 12;
            read_page_from_disk(physical);
            physical -> present = 1;

        }
        else
        {
            page_table* real_page_table = (page_table*) get_system_pointer_from_pde(pt_32); 
            physical = &(real_page_table -> entries[page_table_index]);
        }
        
        tlb_add_pte(&(this -> tlb), virtual_address>>12, physical);
    }
    else
    {
        if(!(physical -> present))
        {
            mmu_raise_page_fault(this);
            addr32 physical_base = ask_kernel_for_frame(physical);
            physical -> base_addr = physical_base >> 12;
            read_page_from_disk(physical);
            physical -> present = 1;
        }
    }

    return physical;
    
}

void mmu_read_from_virtual_address(mmu *this, addr32 virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO implement me
    
    page_table_entry* physical = mmu_get_pte(this, virtual_address, pid);

    //fprintf(stderr, "physical address: %d\n", (addr32)(physical->base_addr));

    if(!physical)
    {
        return;
    }
    

    unsigned int page_number = virtual_address >> 12;
    unsigned int offset = (page_number << 12) ^ virtual_address;
    void* real_address =  get_system_pointer_from_pte(physical) + offset;

    physical -> accessed = 1;
    
    memmove(buffer, real_address, num_bytes);
}

void mmu_write_to_virtual_address(mmu *this, addr32 virtual_address, size_t pid,
                                  const void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO implement me

    
    page_table_entry* physical = mmu_get_pte(this, virtual_address, pid);

    //fprintf(stderr, "physical address: %d\n", (addr32)(physical));

    if(!physical)
    {
        return;
    }

    unsigned int page_number = virtual_address >> 12;
    unsigned int offset = (page_number << 12) ^ virtual_address;
    void* real_address =  get_system_pointer_from_pte(physical) + offset;

    physical -> accessed = 1;
    physical -> dirty = 1;


    memmove(real_address, buffer, num_bytes);
}

void mmu_tlb_miss(mmu *this) {
    this->num_tlb_misses++;
}

void mmu_raise_page_fault(mmu *this) {
    this->num_page_faults++;
}

void mmu_raise_segmentation_fault(mmu *this) {
    this->num_segmentation_faults++;
}

void mmu_add_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    addr32 page_directory_address = ask_kernel_for_frame(NULL);
    this->page_directories[pid] =
        (page_directory *)get_system_pointer_from_address(
            page_directory_address);
    page_directory *pd = this->page_directories[pid];
    this->segmentations[pid] = calloc(1, sizeof(vm_segmentations));
    vm_segmentations *segmentations = this->segmentations[pid];

    // Note you can see this information in a memory map by using
    // cat /proc/self/maps
    segmentations->segments[STACK] =
        (vm_segmentation){.start = 0xBFFFE000,
                          .end = 0xC07FE000, // 8mb stack
                          .permissions = READ | WRITE,
                          .grows_down = true};

    segmentations->segments[MMAP] =
        (vm_segmentation){.start = 0xC07FE000,
                          .end = 0xC07FE000,
                          // making this writeable to simplify the next lab.
                          // todo make this not writeable by default
                          .permissions = READ | EXEC | WRITE,
                          .grows_down = true};

    segmentations->segments[HEAP] =
        (vm_segmentation){.start = 0x08072000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[BSS] =
        (vm_segmentation){.start = 0x0805A000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[DATA] =
        (vm_segmentation){.start = 0x08052000,
                          .end = 0x0805A000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[TEXT] =
        (vm_segmentation){.start = 0x08048000,
                          .end = 0x08052000,
                          .permissions = READ | EXEC,
                          .grows_down = false};

    // creating a few mappings so we have something to play with (made up)
    // this segment is made up for testing purposes
    segmentations->segments[TESTING] =
        (vm_segmentation){.start = PAGE_SIZE,
                          .end = 3 * PAGE_SIZE,
                          .permissions = READ | WRITE,
                          .grows_down = false};
    // first 4 mb is bookkept by the first page directory entry
    page_directory_entry *pde = &(pd->entries[0]);
    // assigning it a page table and some basic permissions
    pde->base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
    pde->present = true;
    pde->read_write = true;
    pde->user_supervisor = true;

    // setting entries 1 and 2 (since each entry points to a 4kb page)
    // of the page table to point to our 8kb of testing memory defined earlier
    for (int i = 1; i < 3; i++) {
        page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
        page_table_entry *pte = &(pt->entries[i]);
        pte->base_addr = (ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
        pte->present = true;
        pte->read_write = true;
        pte->user_supervisor = true;
    }
}

void mmu_remove_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    // example of how to BFS through page table tree for those to read code.
    page_directory *pd = this->page_directories[pid];
    if (pd) {
        for (size_t vpn1 = 0; vpn1 < NUM_ENTRIES; vpn1++) {
            page_directory_entry *pde = &(pd->entries[vpn1]);
            if (pde->present) {
                page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
                for (size_t vpn2 = 0; vpn2 < NUM_ENTRIES; vpn2++) {
                    page_table_entry *pte = &(pt->entries[vpn2]);
                    if (pte->present) {
                        void *frame = (void *)get_system_pointer_from_pte(pte);
                        return_frame_to_kernel(frame);
                    }
                    remove_swap_file(pte);
                }
                return_frame_to_kernel(pt);
            }
        }
        return_frame_to_kernel(pd);
    }

    this->page_directories[pid] = NULL;
    free(this->segmentations[pid]);
    this->segmentations[pid] = NULL;

    if (this->curr_pid == pid) {
        tlb_flush(&(this->tlb));
    }
}

void mmu_delete(mmu *this) {
    for (size_t pid = 0; pid < MAX_PROCESS_ID; pid++) {
        mmu_remove_process(this, pid);
    }

    tlb_delete(this->tlb);
    free(this);
    remove_swap_files();
}
