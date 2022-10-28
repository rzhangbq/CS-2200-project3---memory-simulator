#include "proc.h"
#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 3 --------------------------------------
 * Checkout PDF section 4 for this problem
 *
 * This function gets called every time a new process is created.
 * You will need to allocate a frame for the process's page table using the
 * free_frame function. Then, you will need update both the frame table and
 * the process's PCB.
 *
 * @param proc pointer to process that is being initialized
 *
 * HINTS:
 *      - pcb_t: struct defined in pagesim.h that is a process's PCB.
 *      - You are not guaranteed that the memory returned by the free frame allocator
 *      is empty - an existing frame could have been evicted for our new page table.
 * ----------------------------------------------------------------------------------
 */
void proc_init(pcb_t *proc)
{
    // TODO: initialize proc's page table.
    // finding a free physical frame number
    pfn_t FPFN = free_frame();
    // getting the frame table entry
    fte_t *FTE = &frame_table[FPFN];
    // updating the frame table entry
    FTE->mapped = 1;
    FTE->process = proc;
    // protecting the page table
    FTE->protected = 1;

    // initialize page table
    // zero-initialize the page table
    memset(mem + (FPFN * PAGE_SIZE), 0, PAGE_SIZE);
    // set ptbr to the free frame PFN
    proc->saved_ptbr = FPFN;
}

/**
 * --------------------------------- PROBLEM 4 --------------------------------------
 * Checkout PDF section 5 for this problem
 *
 * Switches the currently running process to the process referenced by the proc
 * argument.
 *
 * Every process has its own page table, as you allocated in proc_init. You will
 * need to tell the processor to use the new process's page table.
 *
 * @param proc pointer to process to become the currently running process.
 *
 * HINTS:
 *      - Look at the global variables defined in pagesim.h. You may be interested in
 *      the definition of pcb_t as well.
 * ----------------------------------------------------------------------------------
 */
void context_switch(pcb_t *proc)
{
    // TODO: update any global vars and proc's PCB to match the context_switch.
    // update ptbr
    PTBR = proc->saved_ptbr;
    // switch current process
    current_process = proc;
}

/**
 * --------------------------------- PROBLEM 8 --------------------------------------
 * Checkout PDF section 8 for this problem
 *
 * When a process exits, you need to free any pages previously occupied by the
 * process.
 *
 * HINTS:
 *      - If the process has swapped any pages to disk, you must call
 *      swap_free() using the page table entry pointer as a parameter.
 *      - If you free any protected pages, you must also clear their"protected" bits.
 * ----------------------------------------------------------------------------------
 */
void proc_cleanup(pcb_t *proc)
{
    // affected process PTBR
    pfn_t evicted_PTBR = proc->saved_ptbr;
    // affected process page table
    pte_t *evicted_pgtable = (pte_t *)(mem + (evicted_PTBR * PAGE_SIZE));

    // TODO: Iterate the proc's page table and clean up each valid page
    for (size_t i = 0; i < NUM_PAGES; i++)
    {
        // evicted page table entry
        pte_t *evicted_PTE = &evicted_pgtable[i];

        // if this page is swapped
        if (swap_exists(evicted_PTE))
            swap_free(evicted_PTE);
        // if this page is valid
        if (evicted_PTE->valid)
        {
            // evicted physical frame number
            pfn_t evicted_PFN = evicted_PTE->pfn;
            // get the frame table entry of the evicted frame:
            fte_t *FTE = &frame_table[evicted_PFN];
            if (FTE->protected)
                printf("freeing protected frame");
            // clearing frame table
            FTE->mapped = 0;
            FTE->process = 0;
            FTE->vpn = 0;
            FTE->referenced = 0;
            // clearing page table entry
            evicted_PTE->valid = 0;
        }
    }
    // clear the page table
    // get the frame table entry of the page table:
    fte_t *FTE = &frame_table[evicted_PTBR];
    // clearing frame table
    FTE->protected = 0;
    FTE->mapped = 0;
    FTE->process = 0;
    FTE->vpn = 0;
    FTE->referenced = 0;
}

#pragma GCC diagnostic pop
