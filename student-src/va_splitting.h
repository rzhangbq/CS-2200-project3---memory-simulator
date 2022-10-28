#pragma once
#include "stdio.h"
#include "mmu.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 1 --------------------------------------
 * Checkout PDF Section 3 For this Problem
 *
 * Split the virtual address into its virtual page number and offset.
 *
 * HINT:
 *      -Examine the global defines in pagesim.h, which will be necessary in
 *      implementing these functions.
 * ----------------------------------------------------------------------------------
 */
static inline vpn_t vaddr_vpn(vaddr_t addr)
{
    // TODO: return the VPN from virtual address addr.
    // validate addr
    if (!(addr <= (1 << VADDR_LEN)))
        printf("invalid addr");
    if (!(addr >= 0))
        printf("invalid addr");
    // get the vpn
    // right shift the addr by the offset length
    vpn_t vpn = addr >> OFFSET_LEN;
    return vpn;
}

static inline uint16_t vaddr_offset(vaddr_t addr)
{
    // TODO: return the offset into the frame from virtual address addr.
    // validate addr
    if (!(addr <= (1 << VADDR_LEN)))
        printf("invalid addr");
    if (!(addr >= 0))
        printf("invalid addr");
    // get the offset
    // get OFFSET_LEN bits of 1s
    uint16_t ones = (1 << OFFSET_LEN) - 1;
    uint16_t offset = addr & ones;
    return offset;
}

#pragma GCC diagnostic pop
