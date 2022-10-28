#include "types.h"
#include "pagesim.h"
#include "mmu.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);

pfn_t last_evicted = 0;

/**
 * --------------------------------- PROBLEM 7 --------------------------------------
 * Checkout PDF section 7 for this problem
 *
 * Make a free frame for the system to use. You call the select_victim_frame() method
 * to identify an "available" frame in the system (already given). You will need to
 * check to see if this frame is already mapped in, and if it is, you need to evict it.
 *
 * `free_frame()` does not zero-initialize the frame
 *
 * @return victim_pfn: a phycial frame number to a free frame be used by other functions.
 *
 * HINTS:
 *      - When evicting pages, remember what you checked for to trigger page faults
 *      in mem_access
 *      - If the page table entry has been written to before, you will need to use
 *      swap_write() to save the contents to the swap queue.
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void)
{
    pfn_t victim_pfn;
    victim_pfn = select_victim_frame();

    // get the frame table entry of the evicted frame:
    fte_t *FTE = &frame_table[victim_pfn];
    if (FTE->protected)
        printf("freeing protected frame");

    // TODO: evict any mapped pages.
    if (FTE->mapped)
    {
        // get VPN to be evicted
        vpn_t evicted_VPN = FTE->vpn;

        // affected process
        pcb_t *evicted_process = FTE->process;
        // affected process PTBR
        pfn_t evicted_PTBR = evicted_process->saved_ptbr;
        // affected process page table
        pte_t *evicted_pgtable = (pte_t *)(mem + (evicted_PTBR * PAGE_SIZE));
        // evicted page table entry
        pte_t *evicted_PTE = &evicted_pgtable[evicted_VPN];
        // evicted physical frame number
        pfn_t evicted_PFN = evicted_PTE->pfn;

        // update PTE
        // if page is dirty:
        if (evicted_PTE->dirty)
        {
            stats.writebacks += 1;
            swap_write(evicted_PTE, &mem[evicted_PFN * PAGE_SIZE]);
            evicted_PTE->dirty = 0;
        }
        // clear the valid bit
        evicted_PTE->valid = 0;
    }
    // clearing frame table
    FTE->mapped = 0;
    FTE->process = 0;
    FTE->vpn = 0;
    FTE->referenced = 0;

    return victim_pfn;
}

/**
 * --------------------------------- PROBLEM 9 --------------------------------------
 * Checkout PDF section 7, 9, and 11 for this problem
 *
 * Finds a free physical frame. If none are available, uses either a
 * randomized, FCFS, or clocksweep algorithm to find a used frame for
 * eviction.
 *
 * @return The physical frame number of a victim frame.
 *
 * HINTS:
 *      - Use the global variables MEM_SIZE and PAGE_SIZE to calculate
 *      the number of entries in the frame table.
 *      - Use the global last_evicted to keep track of the pointer into the frame table
 * ----------------------------------------------------------------------------------
 */
pfn_t select_victim_frame()
{
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++)
    {
        if (!frame_table[i].protected && !frame_table[i].mapped)
        {
            return i;
        }
    }

    // RANDOM implemented for you.
    if (replacement == RANDOM)
    {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t unprotected_found = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++)
        {
            if (!frame_table[i].protected)
            {
                unprotected_found = i;
                if (prng_rand() % 2)
                {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (unprotected_found < NUM_FRAMES)
        {
            return unprotected_found;
        }
    }
    else if (replacement == FIFO)
    {
        // TODO: Implement the FIFO algorithm here
        
        // loop through the frame table for a not protected frame
        // initialize FTE
        fte_t *FTE;

        while (1)
        {
            // increment last_evicted
            last_evicted++;
            
            // going around the mem frame
            last_evicted %= NUM_FRAMES;

            // get the FTE from last_evicted:
            FTE = &frame_table[last_evicted];

            // check not protected
            if (!(FTE->protected))
                return last_evicted;
        }
    }
    else if (replacement == CLOCKSWEEP)
    {
        // TODO: Implement the clocksweep page replacement algorithm here

        // loop through the frame table for a not referenced frame
        // initialize FTE
        fte_t *FTE;

        while (1)
        {
            // increment last_evicted
            last_evicted++;
            
            // going around the mem frame
            last_evicted %= NUM_FRAMES;

            // get the FTE from last_evicted:
            FTE = &frame_table[last_evicted];

            // check protected
            if (FTE->protected)
                continue;

            if (FTE->referenced)
                FTE->referenced = 0;
            else // FTE is has no referenced bit set
                return last_evicted;
        }
    }

    /* If every frame is protected, give up. This should never happen
       on the traces we provide you. */
    panic("System ran out of memory\n");
    exit(1);
}
