#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 6 --------------------------------------
 * Checkout PDF section 7 for this problem
 *
 * Page fault handler.
 *
 * When the CPU encounters an invalid address mapping in a page table, it invokes the
 * OS via this handler. Your job is to put a mapping in place so that the translation
 * can succeed.
 *
 * @param addr virtual address in the page that needs to be mapped into main memory.
 *
 * HINTS:
 *      - You will need to use the global variable current_process when
 *      altering the frame table entry.
 *      - Use swap_exists() and swap_read() to update the data in the
 *      frame as it is mapped in.
 * ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t addr)
{
   // TODO: Get a new frame, then correctly update the page table and frame table
   // splitting the VPN
   vpn_t VPN = vaddr_vpn(addr);
   uint16_t virtual_offset = vaddr_offset(addr);

   // page table
   pte_t *pgtable = (pte_t *)(mem + (PTBR * PAGE_SIZE));
   // page table entry
   pte_t *PTE = &pgtable[VPN];

   // getting a free frame for the PFN
   pfn_t free_frame = free_frame();

   // if the faulting page is in memory, swap back memory
   if (swap_exists(PTE))
      swap_read(PTE, &mem[free_frame * PAGE_SIZE]);
   // if the faulting page is not in memory, clear the space
   else
      memset(mem + (free_frame * PAGE_SIZE), 0, PAGE_SIZE);

   // update PTE
   PTE->pfn = free_frame;
   PTE->valid = 1;

   frame_table[free_frame].mapped = 1;
   frame_table[free_frame].process->pid = current_process->pid;
   frame_table[free_frame].vpn = VPN;
}

#pragma GCC diagnostic pop
