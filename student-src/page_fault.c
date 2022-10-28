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
   stats.page_faults += 1;
   // TODO: Get a new frame, then correctly update the page table and frame table
   // splitting the VPN
   vpn_t VPN = vaddr_vpn(addr);

   // page table
   pte_t *pgtable = (pte_t *)(mem + (PTBR * PAGE_SIZE));
   // page table entry
   pte_t *PTE = &pgtable[VPN];

   // getting a free frame for the PFN
   // free physical frame number
   pfn_t FPFN = free_frame();
   // getting the frame table entry
   fte_t *FTE = &frame_table[FPFN];

   // if the faulting page is in memory, swap back memory
   if (swap_exists(PTE))
      swap_read(PTE, &mem[FPFN * PAGE_SIZE]);
   // if the faulting page is not in memory, clear the space
   else
      memset(mem + (FPFN * PAGE_SIZE), 0, PAGE_SIZE);

   // update PTE
   PTE->pfn = FPFN;
   PTE->valid = 1;

   FTE->mapped = 1;
   FTE->process = current_process;
   FTE->vpn = VPN;
}

#pragma GCC diagnostic pop
